#include "socket.h"
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

int make_udp_socket() {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to construct udp socket.";
    return -1;
  }

  return sockfd;
}

int make_client_sockaddr(struct sockaddr_in *addr, const char *hostname,
                         int port) {
  addr->sin_family = AF_INET;

  struct hostent *host = gethostbyname(hostname);
  if (host == nullptr) {
    std::cerr << "Error: unkown hostname";
    return -1;
  }

  memcpy(&(addr->sin_addr), host->h_addr, host->h_length);
  addr->sin_port = htons(port);

  return 0;
}

int make_server_sockaddr(struct sockaddr_in *addr, int port) {
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  addr->sin_port = htons(port);
  return 0;
}

int send_packet(int sockfd, PacketHeader &header, const char *data,
                const sockaddr_in &dest) {
  // // Send header
  // cout << "About to send header" << endl;
  // if (sendto(sockfd, &header, sizeof(PacketHeader), 0,
  //            (const struct sockaddr *)&dest, sizeof(dest)) < 0) {
  //   perror("Failed to send header.");
  //   return -1;
  // }
  // cout << "Finished sending header" << endl;

  // if (header.length > 0) {
  //   if (sendto(sockfd, data, header.length, 0, (const struct sockaddr
  //   *)&dest,
  //              sizeof(dest)) < 0) {
  //     perror("Failed to send data.");
  //     return -1;
  //   }
  // }
  // TEST

  if (sendto(sockfd, data, header.length, 0, (const struct sockaddr *)&dest,
             sizeof(dest)) < 0) {
    perror("Failed to send data.");
    return -1;
  }
  return 0;
}

int receive_packet(int sockfd, PacketHeader &header, sockaddr_in &src) {
  socklen_t addrLen = sizeof(src);

  // Receive header
  cout << "About to receive header" << endl;
  if (recvfrom(sockfd, &header, sizeof(PacketHeader), 0,
               (struct sockaddr *)&src, &addrLen) < 0) {
    perror("Failed to receive header");
    return -1;
  }
  cout << "Received header" << endl;

  const int DATA_SIZE = ntohl(header.length);
  if (DATA_SIZE > 0) {
    char buffer[DATA_SIZE];
    if (recvfrom(sockfd, buffer, header.length, 0, (struct sockaddr *)&src,
                 &addrLen) < 0) {
      perror("Failed to receive data");
      return -1;
    }
  }

  return 0;
}

int send_packet_header(PacketHeader &packet_header, int sockfd,
                       sockaddr_in &addr) {
  if (sendto(sockfd, &packet_header, sizeof(packet_header), 0,
             (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
    cerr << "Failed to send packet header" << endl;
    return -1;
  }
  return 0;
}

bool receive_packet_header(PacketHeader &packet_header, int sockfd,
                           sockaddr_in &addr) {
  socklen_t addrLen = sizeof(addr);
  ssize_t bytes_received =
      recvfrom(sockfd, &packet_header, sizeof(PacketHeader), MSG_DONTWAIT,
               (struct sockaddr *)&addr, &addrLen);
  return bytes_received == sizeof(packet_header);
}