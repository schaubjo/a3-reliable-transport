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

int calc_new_window_start(
    std::unordered_map<int,
                       std::pair<bool, std::chrono::steady_clock::time_point>>
        packet_timestamps,
    int old_window_start) {

  int i = old_window_start;
  while (true) {
    // Find first index after old_window_start with no received packet
    if (packet_timestamps.find(i) == packet_timestamps.end() ||
        !packet_timestamps[i].first) {
      return i;
    }
    i++;
  }
  std::cout << "Error in calc_new_window_start" << std::endl;
  return -1;
}

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

  // Declare map to record timestamps for each packet sent
  std::unordered_map<int,
                     std::pair<bool, std::chrono::steady_clock::time_point>>
      packet_timestamps;
  auto start_time = std::chrono::steady_clock::now();

  // Send all packets in window
  int window_start = 0;
  int window_end = std::min(WINDOW_SIZE, static_cast<int>(packets.size()));
  for (int i = window_start; i < window_end; i++) {
    send_packet(packets[i], server_addr, sockfd, log);
    packet_timestamps[i] = {false, start_time};
  }

  // Wait for acks
  while (true) {
    Packet packet;
    if (receive_packet(packet, server_addr, sockfd, log) &&
        packet.header.type == ACK) {
      // If an ACK was received
      int acked_seq_num = packet.header.seqNum;
      if (acked_seq_num == window_start) {
        // Can slide window
        packet_timestamps[acked_seq_num].first = true;

        // Get start of new window
        window_start = calc_new_window_start(packet_timestamps, window_start);
        if (window_start == static_cast<int>(packets.size())) {
          std::cout << "ALL PACKETS SENT" << std::endl;
          break;
        }

        // Get end of new window
        int prev_end = window_end;
        window_end = std::min(window_start + WINDOW_SIZE,
                              static_cast<int>(packets.size()));

        start_time = std::chrono::steady_clock::now();
        for (int i = prev_end; i < window_end; i++) {
          // Send packets that were just added to this window
          send_packet(packets[i], server_addr, sockfd, log);
          packet_timestamps[i] = {false, start_time};
        }
      } else if (acked_seq_num > window_start) {
        // TODO: need to check that acked_seq_num is less than window_end?
        // Can't slide window but can mark packet as received in map
        packet_timestamps[acked_seq_num].first = true;
      }
    }

    // Check if any packets have timed out
    for (int i = window_start; i < window_end; i++) {
      bool received = packet_timestamps[i].first;
      auto time_at_send = packet_timestamps[i].second;
      auto elapsed_time = std::chrono::steady_clock::now() - time_at_send;
      if (!received &&
          std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time)
                  .count() > RETRANSMISSION_TIMER) {
        // Resend packet if packet has not yet been acked and 500 ms have passed
        send_packet(packets[i], server_addr, sockfd, log);

        // Reset timer for packet
        packet_timestamps[i].second = std::chrono::steady_clock::now();
      }
    }
  }

  // End connection
  end_connection(server_addr, sockfd, start_seq_num, log);

  // Close log file
  log.close();
  return 0;
}
