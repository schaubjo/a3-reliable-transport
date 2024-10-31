#include "PacketHeader.h"
#include "helper.h"
#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[]) {
  // Parse command line arguments
  if (argc != 6) {
    printf("Usage: %s <receiver_ip> <receiver_port> <window_size> "
           "<input_filename> <log_filename>\n",
           argv[0]);
    return 1;
  }

  const char *IP = argv[1];
  const int PORT = std::stoi(argv[2]);
  const int WINDOW_SIZE = std::stoi(argv[3]);
  const std::string input_filename = argv[4];
  const std::string log_filename = argv[5];

  // Open the file in truncate mode to clear its contents
  std::ofstream log = truncate_log_and_set_append(log_filename);

  // Transform input file into vector of packets
  std::vector<Packet> packets = packet_data_init(input_filename);
  std::cout << packets.size() << std::endl;
  // Socket setup
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to make UDP socket." << std::endl;
    return 1;
  }

  struct sockaddr_in server_addr;

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr =
      inet_addr(IP); // Localhost 127.0.0.1 for testing

  // Generate random initial seq num for START
  int start_seq_num = generate_start_seq_num();

  // Initiate connection
  start_connection(server_addr, sockfd, start_seq_num, log);

  // Send all packets in window
  int window_start = 0;
  int window_end = std::min(WINDOW_SIZE, static_cast<int>(packets.size()));
  for (int i = window_start; i < window_end; i++) {
    send_packet(packets[i], server_addr, sockfd, log);
  }
  // Wait for acks
  auto start_time = std::chrono::steady_clock::now();

  while (window_start < packets.size()) {
    Packet packet;
    if (receive_packet(packet, server_addr, sockfd, log) &&
        packet.header.type == ACK) {
      // If an ACK was received
      int acked_seq_num = packet.header.seqNum;
      if (window_start < acked_seq_num) {
        // Receiver is expecting more data; slide window and send new packets
        if (acked_seq_num == packets.size()) {
          std::cout << "ALL PACKETS SENT" << std::endl;
          // Finish if all packets sent
          break;
        }
        int prev_end = window_end;
        window_start = acked_seq_num;
        window_end = std::min(window_start + WINDOW_SIZE,
                              static_cast<int>(packets.size()));
        for (int i = prev_end; i < window_end; i++) {
          // Send packets that were just added to this window
          send_packet(packets[i], server_addr, sockfd, log);
        }

        // Reset timer
        start_time = std::chrono::steady_clock::now();
      }
    }

    // Check for timeout
    auto elapsed_time = std::chrono::steady_clock::now() - start_time;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time)
            .count() > RETRANSMISSION_TIMER) {
      // Resend packets in window
      for (int i = window_start; i < window_end; i++) {
        send_packet(packets[i], server_addr, sockfd, log);
      }

      // Reset timer
      start_time = std::chrono::steady_clock::now();
    } else {
      // Avoid busy waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // End connection
  end_connection(server_addr, sockfd, start_seq_num, log);

  // Close log file
  log.close();
  return 0;
}
