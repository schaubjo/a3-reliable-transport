#include "PacketHeader.h"
#include "helper.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>

int calc_continuous_packets_received(std::set<int> packets_received) {
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

  std::cout << "Listening..." << std::endl;
  // TODO: Add window stuff
  std::set<int> packets_received;
  std::set<int> started_connections;
  int connection_seq_num = -1;
  int window_start = 0;
  int window_end = window_start + WINDOW_SIZE;
  while (true) {
    // Receive any packet
    Packet packet;
    if (receive_packet(packet, server_addr, sockfd, log)) {
      if (packet.header.type == START) {
        std::cout << "RECEIVER START" << std::endl;

        // Only accept START if no connection in progress or duplicate START
        if (connection_seq_num == -1 ||
            connection_seq_num == packet.header.seqNum) {
          // std::cout << "RECEIVER START 2" << std::endl;

          // Send acknowledgement
          PacketHeader ack_header;
          ack_header.type = htonl(ACK);
          ack_header.length = htonl(0);
          ack_header.seqNum = htonl(packet.header.seqNum);
          ack_header.checksum = htonl(0);
          Packet ack_packet;
          ack_packet.header = ack_header;
          send_packet(ack_packet, server_addr, sockfd, log);

          connection_seq_num = packet.header.seqNum;
        }

      } else if (packet.header.type == DATA && connection_seq_num != -1 &&
                 window_start <= packet.header.seqNum &&
                 packet.header.seqNum < window_end) {
        // TODO: add data somewhere
        packets_received.insert(packet.header.seqNum);

        // Adjust window as necessary
        window_start = calc_continuous_packets_received(packets_received);
        window_end = window_start + WINDOW_SIZE;

        // Send acknowledgement for data
        PacketHeader ack_header;
        ack_header.type = htonl(ACK);
        ack_header.length = htonl(0);
        ack_header.seqNum = htonl(window_start);
        ack_header.checksum = htonl(0);
        Packet ack_packet;
        ack_packet.header = ack_header;
        send_packet(ack_packet, server_addr, sockfd, log);

      } else if (packet.header.type == END) {
        // If the current connection is closing
        // std::cout << packet.header.seqNum << std::endl;
        // std::cout << connection_seq_num << std::endl;
        if (connection_seq_num == packet.header.seqNum) {
          // std::cout << "ERASING" << std::endl;
          // Erase data for this connection
          window_start = 0;
          window_end = window_start + WINDOW_SIZE;
          packets_received.clear();
          connection_seq_num = -1;
        }
        std::cout << "RECEIVER END" << std::endl;
        // Send acknowledgement
        PacketHeader ack_header;
        ack_header.type = htonl(ACK);
        ack_header.length = htonl(0);
        ack_header.seqNum = htonl(packet.header.seqNum);
        ack_header.checksum = htonl(0);
        Packet ack_packet;
        ack_packet.header = ack_header;
        send_packet(ack_packet, server_addr, sockfd, log);
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