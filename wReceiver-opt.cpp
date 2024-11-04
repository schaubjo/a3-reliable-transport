#include "PacketHeader.h"
#include "helper.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>

int calc_continuous_packets_received(
    std::unordered_map<int, Packet> packets_received) {
  int i = 0;
  while (true) {
    if (packets_received.find(i) == packets_received.end()) {
      break;
    }
    i++;
  }
  return i;
}
int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: %s <port-num> <window-size> <output-dir> <log>\n", argv[0]);
    return 1;
  }
  const int port_num = std::stoi(argv[1]);
  const int WINDOW_SIZE = std::stoi(argv[2]);
  const std::string output_dir = argv[3];
  const std::string log_filename = argv[4];

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in server_addr;

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_num);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    close(sockfd);
    return 1;
  }
  // Open the file in truncate mode to clear its contents
  std::ofstream log = truncate_log_and_set_append(log_filename);

  std::filesystem::path output_dir_path =
      std::filesystem::current_path() / output_dir.substr(1);
  if (std::filesystem::create_directory(output_dir_path)) {
    std::cout << "Created output_dir" << std::endl;
  } else {
    std::cout << "output_dir already exists." << std::endl;
  }

  std::cout << "Listening..." << std::endl;
  std::unordered_map<int, Packet> packets_received;

  // Holds the START/END seqNum of the current connection
  int connection_seq_num = -1;
  int connection_count = 0;
  int window_start = 0;
  int window_end = window_start + WINDOW_SIZE;
  while (true) {
    // Try to receive packet
    Packet packet;
    if (receive_packet(packet, server_addr, sockfd, log)) {
      if (packet.header.type == START) {
        std::cout << "RECEIVER START" << std::endl;

        // Only accept START if no connection in progress or sender retries
        if (connection_seq_num == -1 ||
            connection_seq_num == static_cast<int>(packet.header.seqNum)) {
          // Send acknowledgement
          send_ack(server_addr, sockfd, log, packet.header.seqNum);

          connection_seq_num = packet.header.seqNum;
        }

      } else if (packet.header.type == DATA && connection_seq_num != -1 &&
                 static_cast<int>(packet.header.seqNum) < window_end &&
                 valid_checksum(packet)) {
        // Mark packet as received
        packets_received[packet.header.seqNum] = packet;

        // Adjust window as necessary
        window_start = calc_continuous_packets_received(packets_received);
        window_end = window_start + WINDOW_SIZE;

        // Send acknowledgement for data
        send_ack(server_addr, sockfd, log, window_start);
      } else if (packet.header.type == END) {
        // If the current connection is closing
        std::string output_file =
            "FILE-" + std::to_string(connection_count) + ".out";
        if (connection_seq_num == static_cast<int>(packet.header.seqNum)) {
          std::string output_file_path = output_dir_path / output_file;
          write_data(output_file_path, packets_received);

          // Erase data for this connection
          window_start = 0;
          window_end = window_start + WINDOW_SIZE;
          packets_received.clear();
          connection_seq_num = -1;
          connection_count++;
        }
        std::cout << "RECEIVER END" << std::endl;
        // Send acknowledgement
        send_ack(server_addr, sockfd, log, packet.header.seqNum);
      }
    } else {
      // Avoid busy waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // Close the socket
  close(sockfd);
  return 0;
}