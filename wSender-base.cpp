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
  std::ofstream log(log_filename, std::ios_base::trunc);
  if (!log.is_open()) {
    std::cerr << "Failed to truncate log at start." << std::endl;
    return 1;
  }
  log.close(); // Close after clearing

  // Reopen the log file in append mode for logging
  log.open(log_filename, std::ios_base::app);
  if (!log.is_open()) {
    std::cerr << "Failed to open log for appending." << std::endl;
    return 1;
  }
  // Transform input file into vector of packets
  std::vector<Packet> packets = packet_data_init(input_filename);

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

  // Send data packets until all have been received

  // int window_start = 0;
  // while (true) {
  //   // Send all packets in window
  //   int window_end =
  //       std::min(window_start + WINDOW_SIZE,
  //       static_cast<int>(packets.size()));
  //   for (int i = window_start; i < window_end; i++) {
  //     send_packet(packets[i], server_addr, sockfd);
  //   }

  //   // Wait for acks
  //   auto start_time = std::chrono::steady_clock::now();
  //   while (true) {
  //     Packet packet;
  //     if (receive_packet(packet, server_addr, sockfd) &&
  //         packet.header.type == ACK) {
  //       // If an ACK was received
  //       int acked_seq_num = packet.header.seqNum;
  //       if (window_start > acked_seq_num && acked_seq_num < window_end) {
  //         // TODO: Receiver is expecting more data; slide window and send new
  //         // packets
  //       }
  //     }

  //     // Check if timeout has elapsed
  //     auto elapsed_time = std::chrono::steady_clock::now() - start_time;
  //     if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time)
  //             .count() > RETRANSMISSION_TIMER) {
  //       // Resend all packets in the current window
  //       for (int i = window_start; i < window_end; i++) {
  //         send_packet(packets[i], server_addr, sockfd);
  //       }
  //       start_time = std::chrono::steady_clock::now(); // Reset timer
  //     }
  //   }
  // }
  // End connection
  end_connection(server_addr, sockfd, start_seq_num, log);

  // Close log file
  log.close();
  return 0;
}
