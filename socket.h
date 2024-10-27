#ifndef SOCKET_H
#define SOCKET_H

#include "PacketHeader.h"
#include <netinet/in.h>

int make_udp_socket();

int make_client_sockaddr(struct sockaddr_in *addr, const char *hostname,
                         int port);

int make_server_sockaddr(struct sockaddr_in *addr, int port);

int send_packet(int sockfd, PacketHeader &packet, const char *data,
                const sockaddr_in &dest);

int receive_packet(int sockfd, PacketHeader &header, sockaddr_in &src);
#endif
