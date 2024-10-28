#include "PacketHeader.h"
#include "socket.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

#define MAX_PACKET_SIZE 1472
#define START 0
#define END 1
#define DATA 2
#define ACK 3

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: %s <port-num> <window-size> <output-dir> <log>\n", argv[0]);
    return 1;
  }
  int port_num = std::stoi(argv[1]);
  int window_size = std::stoi(argv[2]);
  std::string output_dir = argv[3];
  std::string log = argv[4];

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in server_addr, client_addr;

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;         // IPv4
  server_addr.sin_port = htons(port_num);   // Port
  server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces

  // Bind the socket
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    close(sockfd);
    return 1;
  }

  std::cout << "Listening..." << std::endl;

  while (true) {
    // Wait until START message is received to initiate connection
    bool connection_received = false;
    while (!connection_received) {
      PacketHeader start_header;
      if (receive_packet_header(start_header, sockfd, server_addr) &&
          ntohl(start_header.type == START)) {
        std::cout << "Received START message" << std::endl;
        connection_received = true;
        // Send ACK
        PacketHeader ack_header;
        ack_header.type = htonl(ACK);
        ack_header.length = htonl(0);
        ack_header.seqNum = start_header.seqNum;
        ack_header.checksum = htonl(0);
        std::cout << "Sending ACK for START" << std::endl;
        if (send_packet_header(ack_header, sockfd, server_addr) < 0) {
          return 1;
        }
      }

      // Busy waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Receive packets until END message arrives
    std::cout << "Waiting for data..." << std::endl;
    bool end = false;
    while (!end) {
      PacketHeader packet_header;
      if (receive_packet_header(packet_header, sockfd, server_addr)) {
        if (ntohl(packet_header.type == END)) {
          // TODO: check seqnum to be same as start
        }
        if (ntohl(packet_header.type == DATA)) {
        }
      }
      // Busy waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // Close the socket
  close(sockfd);
  return 0;
}