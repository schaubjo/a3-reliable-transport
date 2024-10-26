#include "socket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int make_udp_socket() {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to construct udp socket.";
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