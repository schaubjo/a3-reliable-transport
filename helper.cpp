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

void start_connection(sockaddr_in &server_addr, int sockfd, int startSeqNum) {
  // Initialize start packet header contents
  PacketHeader start_packet_header;
  start_packet_header.type = htonl(START);
  start_packet_header.length = htonl(0);
  start_packet_header.seqNum = htonl(startSeqNum);
  start_packet_header.checksum = htonl(31); // TODO: add crc

  Packet start_packet;
  start_packet.header = start_packet_header;

  bool acked = false;
  while (!acked) {
    // Send START message until acknowledged
    std::cout << "Sending START" << std::endl;
    send_packet(start_packet, server_addr, sockfd);

    // Resend if not acknowledged within 500 ms
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
      // Check for ACK
      Packet ack_packet;
      if (receive_packet(ack_packet, server_addr, sockfd) &&
          ack_packet.header.type == ACK &&
          ack_packet.header.seqNum == htonl(start_packet.header.seqNum)) {
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
        break;
      }
    }
  }
}

void end_connection(sockaddr_in &server_addr, int sockfd, int startSeqNum) {
  std::cout << "Ending connection..." << std::endl;
  // Initialize start packet header contents
  PacketHeader end_packet_header;
  end_packet_header.type = htonl(START);
  end_packet_header.length = htonl(0);
  end_packet_header.seqNum = htonl(startSeqNum);
  end_packet_header.checksum = htonl(31); // TODO: add crc

  Packet end_packet;
  end_packet.header = end_packet_header;

  bool acked = false;
  while (!acked) {
    // Send START message until acknowledged
    std::cout << "Sending END" << std::endl;
    send_packet(end_packet, server_addr, sockfd);

    // Resend if not acknowledged within 500 ms
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
      // Check for ACK
      Packet ack_packet;
      if (receive_packet(ack_packet, server_addr, sockfd) &&
          ack_packet.header.type == ACK &&
          ack_packet.header.seqNum == htonl(end_packet.header.seqNum)) {
        std::cout << "ACK received for END" << std::endl;
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
        break;
      }
    }
  }
  // Close the socket
  close(sockfd);
}

void send_packet(Packet &packet, sockaddr_in &addr, int sockfd) {
  // Declare buffer
  const ssize_t PACKET_SIZE = PACKET_HEADER_SIZE + packet.header.length;
  char buffer[PACKET_SIZE];

  // Copy packet header into buffer
  memcpy(buffer, &packet.header, sizeof(PacketHeader));

  // Copy packet data into buffer
  memcpy(buffer + sizeof(PacketHeader), packet.data, packet.header.length);

  // Send packet to receiver
  ssize_t bytes_sent = sendto(sockfd, buffer, PACKET_SIZE, 0,
                              (const struct sockaddr *)&addr, sizeof(addr));
  if (bytes_sent != PACKET_SIZE) {
    cerr << "Failed to send all bytes in packet." << endl;
  }
}
bool receive_packet(Packet &packet, sockaddr_in &addr, int sockfd) {
  char buffer[MAX_PACKET_SIZE];
  socklen_t addr_len = sizeof(addr);

  ssize_t bytes_received =
      recvfrom(sockfd, buffer, MAX_PACKET_SIZE, MSG_DONTWAIT,
               (struct sockaddr *)&addr, &addr_len);
  if (bytes_received <= 0) {
    // Nothing to receive
    return false;
  }

  // Get header from buffer
  memcpy(&packet.header, buffer, sizeof(PacketHeader));

  // Fix network byte order
  packet.header.type = ntohl(packet.header.type);
  packet.header.seqNum = ntohl(packet.header.seqNum);
  packet.header.length = ntohl(packet.header.length);
  packet.header.checksum = ntohl(packet.header.checksum);
  cout << "Received packet header type: " << packet.header.type << endl;
  cout << "Received packet header seqNum: " << packet.header.seqNum << endl;
  cout << "Received packet header length: " << packet.header.length << endl;
  cout << "Received packet header checkSum: " << packet.header.checksum << endl;

  // If this is a data message, get data from buffer
  int data_length = packet.header.length;
  if (data_length > 0) {
    memcpy(packet.data, buffer + sizeof(PacketHeader), data_length);
  }
  return true;
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
