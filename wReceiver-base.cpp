#include "PacketHeader.h"
#include "helper.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: %s <port-num> <window-size> <output-dir> <log>\n", argv[0]);
    return 1;
  }
  const int port_num = std::stoi(argv[1]);
  const int window_size = std::stoi(argv[2]);
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

  std::cout << "Listening..." << std::endl;
  // TODO: Add window stuff
  while (true) {
    // Receive any packet
    Packet packet;
    if (receive_packet(packet, server_addr, sockfd, log)) {
      if (packet.header.type == START) {
        // Send acknowledgement
        PacketHeader ack_header;
        ack_header.type = htonl(ACK);
        ack_header.length = htonl(0);
        ack_header.seqNum = htonl(packet.header.seqNum);
        ack_header.checksum = htonl(0);
        Packet ack_packet;
        ack_packet.header = ack_header;
        std::cout << "Sending ACK for START" << std::endl;
        send_packet(ack_packet, server_addr, sockfd, log);
      } else if (packet.header.type == DATA) {

      } else if (packet.header.type == END) {
        // Send acknowledgement
        PacketHeader ack_header;
        ack_header.type = htonl(ACK);
        ack_header.length = htonl(0);
        ack_header.seqNum = htonl(packet.header.seqNum);
        ack_header.checksum = htonl(0);
        Packet ack_packet;
        ack_packet.header = ack_header;
        std::cout << "Sending ACK for END" << std::endl;
        send_packet(ack_packet, server_addr, sockfd, log);

        break; // TEMP
      }
    } else {
      // Busy waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // Close the socket
  close(sockfd);
  return 0;
}