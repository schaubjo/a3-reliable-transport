#include "helper.h"
#include "PacketHeader.h"
#include "crc32.h"
#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <random>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;

void start_connection(sockaddr_in &server_addr, int sockfd, int start_seq_num,
                      ofstream &log) {
  // Initialize start packet header contents
  PacketHeader start_packet_header;
  start_packet_header.type = htonl(START);
  start_packet_header.length = htonl(0);
  start_packet_header.seqNum = htonl(start_seq_num);
  start_packet_header.checksum = htonl(0);

  Packet start_packet;
  start_packet.header = start_packet_header;

  bool acked = false;
  while (!acked) {
    // Send START message until acknowledged
    std::cout << "Sending START" << std::endl;
    send_packet(start_packet, server_addr, sockfd, log);

    // Resend if not acknowledged within 500 ms
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
      // Check for ACK
      Packet ack_packet;
      if (receive_packet(ack_packet, server_addr, sockfd, log) &&
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

void end_connection(sockaddr_in &server_addr, int sockfd, int start_seq_num,
                    ofstream &log) {
  std::cout << "Ending connection..." << std::endl;
  // Initialize start packet header contents
  PacketHeader end_packet_header;
  end_packet_header.type = htonl(END);
  end_packet_header.length = htonl(0);
  end_packet_header.seqNum = htonl(start_seq_num);
  end_packet_header.checksum = htonl(0);

  Packet end_packet;
  end_packet.header = end_packet_header;

  bool acked = false;
  while (!acked) {
    // Send START message until acknowledged
    std::cout << "Sending END" << std::endl;
    send_packet(end_packet, server_addr, sockfd, log);

    // Resend if not acknowledged within 500 ms
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
      // Check for ACK
      Packet ack_packet;
      if (receive_packet(ack_packet, server_addr, sockfd, log) &&
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

void send_packet(Packet &packet, sockaddr_in &addr, int sockfd, ofstream &log) {
  // Declare buffer
  const ssize_t PACKET_SIZE = PACKET_HEADER_SIZE + ntohl(packet.header.length);
  char buffer[PACKET_SIZE];

  // Copy packet header into buffer
  memcpy(buffer, &packet.header, sizeof(PacketHeader));

  // Copy packet data into buffer
  memcpy(buffer + sizeof(PacketHeader), packet.data,
         ntohl(packet.header.length));

  // Send packet to receiver
  ssize_t bytes_sent = sendto(sockfd, buffer, PACKET_SIZE, 0,
                              (const struct sockaddr *)&addr, sizeof(addr));
  if (bytes_sent != PACKET_SIZE) {
    cerr << "Failed to send all bytes in packet." << endl;
  }

  // Log info
  log << htonl(packet.header.type) << " " << htonl(packet.header.seqNum) << " "
      << htonl(packet.header.length) << " " << htonl(packet.header.checksum)
      << endl;
}

bool receive_packet(Packet &packet, sockaddr_in &addr, int sockfd,
                    ofstream &log) {
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
  if (packet.header.type == START) {
    cout << "START packet received" << endl;
  } else if (packet.header.type == ACK) {
    cout << "ACK packet received" << endl;
  } else if (packet.header.type == DATA) {
    cout << "DATA packet received" << endl;
  } else if (packet.header.type == END) {
    cout << "END packet received" << endl;
  }
  cout << "Received packet header seqNum: " << packet.header.seqNum << endl;
  cout << "Received packet header length: " << packet.header.length << endl;
  cout << "Received packet header checkSum: " << packet.header.checksum << endl;

  // If this is a data message, get data from buffer
  int data_length = packet.header.length;
  if (data_length > 0) {
    memcpy(packet.data, buffer + sizeof(PacketHeader), data_length);
  }

  // Log info
  log << packet.header.type << " " << packet.header.seqNum << " "
      << packet.header.length << " " << packet.header.checksum << endl;
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
      PacketHeader data_header;
      data_header.type = htonl(DATA);
      data_header.seqNum = htonl(seqNum++);
      data_header.length = htonl(num_bytes_read);
      data_header.checksum = htonl(crc32(buffer, num_bytes_read));

      Packet packet;
      packet.header = data_header;

      // Copy the data into struct
      std::memcpy(packet.data, buffer, num_bytes_read);

      // Add packet to vector
      packets.push_back(packet);
    }
  }
  return packets;
}

int generate_start_seq_num() {
  // Generate random number between 0 and 100
  random_device rd;
  mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  uniform_int_distribution<int> dist(0, 100);
  return dist(gen);
}

ofstream truncate_log_and_set_append(string log_filename) {
  ofstream log(log_filename, ios_base::trunc);
  if (!log.is_open()) {
    cerr << "Failed to truncate log at start." << endl;
  }
  log.close(); // Close after clearing

  // Reopen the log file in append mode for logging
  log.open(log_filename, ios_base::app);
  if (!log.is_open()) {
    cerr << "Failed to open log for appending." << endl;
  }

  return log;
}