#include "helper.h"
#include "PacketHeader.h"
#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
using namespace std;

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

void end_connection(int sockfd, sockaddr_in &server_addr) {
  std::cout << "Ending connection..." << std::endl;
  // TODO: Send END packet and wait for ACK

  // Close the socket
  close(sockfd);
}

void send_packet_header(PacketHeader &packet_header, int sockfd,
                        sockaddr_in &addr) {
  if (sendto(sockfd, &packet_header, sizeof(packet_header), 0,
             (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
    cerr << "Failed to send packet header" << endl;
  }
}

bool receive_packet_header(PacketHeader &packet_header, int sockfd,
                           sockaddr_in &addr) {
  socklen_t addrLen = sizeof(addr);
  ssize_t bytes_received =
      recvfrom(sockfd, &packet_header, sizeof(PacketHeader), MSG_DONTWAIT,
               (struct sockaddr *)&addr, &addrLen);
  return bytes_received == sizeof(packet_header);
}

vector<Packet> packet_data_init(const string &filename) {
  std::vector<Packet> packets;
  std::ifstream file(filename, std::ios::binary);

  if (!file) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return packets;
  }

  int seqNum = INITIAL_SEQNUM;
  char buffer[MAX_PACKET_SIZE - PACKET_HEADER_SIZE];

  while (file) {
    file.read(buffer, sizeof(buffer));
    std::streamsize num_bytes_read = file.gcount();

    if (num_bytes_read > 0) {
      Packet packet;
      packet.header.type = DATA;
      packet.header.seqNum = seqNum++;
      packet.header.length = num_bytes_read;
      packet.header.checksum = 0; // TODO: calculate checksum

      // Copy the data into struct
      std::memcpy(packet.data, buffer, num_bytes_read);

      // Add packet to vector
      packets.push_back(packet);
    }
  }
  return packets;
}