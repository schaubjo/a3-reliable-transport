#ifndef HELPER_H
#define HELPER_H

#include "PacketHeader.h"
#include <netinet/in.h>
#include <string>
#include <vector>

struct Packet {
  PacketHeader header;
  char data[1456]; // max payload
};

void send_packet_header(PacketHeader &packet_header, int sockfd,
                        sockaddr_in &addr);

bool receive_packet_header(PacketHeader &packet_header, int sockfd,
                           sockaddr_in &addr);

std::vector<Packet> packet_data_init(std::string &filename);
#endif
