#ifndef HELPER_H
#define HELPER_H

#include "PacketHeader.h"
#include <netinet/in.h>

void send_packet_header(PacketHeader &packet_header, int sockfd,
                        sockaddr_in &addr);

bool receive_packet_header(PacketHeader &packet_header, int sockfd,
                           sockaddr_in &addr);
#endif
