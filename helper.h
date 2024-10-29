#ifndef HELPER_H
#define HELPER_H

#include "PacketHeader.h"
#include <netinet/in.h>
#include <string>
#include <vector>

constexpr int MAX_PACKET_SIZE = 1472;
constexpr int PACKET_HEADER_SIZE = 16;
constexpr int INITIAL_SEQNUM = 0;
constexpr int START = 0;
constexpr int END = 1;
constexpr int DATA = 2;
constexpr int ACK = 3;
const int RETRANSMISSION_TIMER = 500; // ms

struct Packet {
  PacketHeader header;
  char data[1456]; // max payload
};

void start_connection(sockaddr_in &server_addr, int sockfd, int start_seq_num);

void end_connection(sockaddr_in &server_addr, int sockfd, int start_seq_num);

void send_packet(Packet &packet, sockaddr_in &addr, int sockfd);

bool receive_packet(Packet &packet, sockaddr_in &addr, int sockfd);

std::vector<Packet> packet_data_init(const std::string &filename);

int generate_start_seq_num();
#endif
