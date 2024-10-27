
#include "PacketHeader.h"
#include "crc32.h"
#include "socket.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#define MAX_PACKET_SIZE 1472
#define START 0
#define END 1
#define DATA 2
#define ACK 3

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: %s <port-num> <window-size> <output-dir> <log>\n", argv[0]);
    return 1;
  }
  int port_num = stoi(argv[1]);
  int window_size = stoi(argv[2]);
  string output_dir = argv[3];
  string log = argv[4];

  //   ofstream log_file(log, ofstream::app);
  //   log_file.close();

  // // Create UDP socket
  // int sockfd = make_udp_socket();
  // if (sockfd < 0) {
  //   cerr << "Failed to create receiver UDP socket." << endl;
  // }

  // // Set the "reuse port" socket option
  // int yes = 1;
  // setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  // // Create sockaddr_in for port and bind to it
  // struct sockaddr_in addr;
  // if (make_server_sockaddr(&addr, port_num) == -1) {
  //   cerr << "Failed to make server sockaddr" << endl;
  //   return -1;
  // }

  // bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

  // // Receive a packet
  // PacketHeader header;
  // while (true) {
  //   if (receive_packet(sockfd, header, addr) < 0) {
  //     return -1;
  //   }
  //   cout << "Successfully received packet" << endl;
  //   cout << "Checksum: " << header.checksum << endl;
  // }
}