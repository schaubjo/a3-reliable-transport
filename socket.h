#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>

int make_udp_socket();

int make_client_sockaddr(struct sockaddr_in *addr, const char *hostname,
                         int port);

#endif
