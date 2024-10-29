#include "helper.h"
#include "PacketHeader.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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