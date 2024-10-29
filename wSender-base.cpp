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
const int RETRANSMISSION_TIMER = 500; // ms

void start_connection(int sockfd, sockaddr_in &server_addr) {
  // Initialize start packet header contents
  PacketHeader start_packet;
  start_packet.type = htonl(START);
  start_packet.length = htonl(0);
  start_packet.seqNum = htonl(4);    // TODO: change to rand()
  start_packet.checksum = htonl(31); // TODO: add crc

  // Send START message
  bool acked = false;

  while (!acked) {
    std::cout << "Sending START" << std::endl;
    send_packet_header(start_packet, sockfd, server_addr);

    // Resend START if not acknowledged within 500 ms
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
      // Check for ACK
      PacketHeader ack_header;
      if (receive_packet_header(ack_header, sockfd, server_addr) &&
          ntohl(ack_header.type) == ACK &&
          ntohl(ack_header.seqNum) == htonl(start_packet.seqNum)) {
        std::cout << "ACK received for START" << std::endl;
        acked = true;
        break;
      }

      // Check if need to retransmit
      auto current_time = std::chrono::steady_clock::now();
      auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            current_time - start_time)
                            .count();
      if (elapsed_ms >= RETRANSMISSION_TIMER) {
        std::cout << "Retransmitting packet..." << std::endl;
        break; // Exit the inner loop to retransmit the packet
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 6) {
    printf("Usage: %s <receiver_ip> <receiver_port> <window_size> "
           "<input_filename> <log_filename>\n",
           argv[0]);
    return 1;
  }

  const char *receiver_ip = argv[1];
  int receiver_port = std::stoi(argv[2]);
  int window_size = std::stoi(argv[3]);
  std::string input_filename = argv[4];
  std::string log_filename = argv[5];

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to make UDP socket." << std::endl;
    return 1;
  }

  struct sockaddr_in server_addr;
  socklen_t addr_len = sizeof(server_addr);

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(receiver_port);
  server_addr.sin_addr.s_addr =
      inet_addr(receiver_ip); // Localhost 127.0.0.1 for testing

  // Send START packet to initiate connection
  start_connection(sockfd, server_addr);

  // Close the socket
  close(sockfd);
  return 0;
}
