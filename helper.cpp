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

vector<Packet> packet_data_init(string &filename) {
  std::vector<Packet> packets;
  std::ifstream file(filename, std::ios::binary);

  if (!file) {
    std::cerr << "Error: Could not open file " << filename << std::endl;
    return packets;
  }

  int seqNum = 0;
  char buffer[1456]; // 1472 packet bytes - 16 header bytes = 1456 data bytes

  while (file) {
    file.read(buffer, sizeof(buffer));
    std::streamsize num_bytes_read = file.gcount();

    if (num_bytes_read > 0) {
      Packet packet;
      packet.header.type = 2;
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