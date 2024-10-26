
#include "PacketHeader.h"
#include "crc32.h"
#include "socket.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#define MAX_PACKET_SIZE 1472
#define START 0
#define END 1
#define DATA 2
#define ACK 3

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 6) {
    printf("Usage: %s <receiver_ip> <receiver_port> <window_size> "
           "<input_filename> <log_filename>\n",
           argv[0]);
    return 1;
  }

  string receiver_ip = argv[1];
  int receiver_port = stoi(argv[2]);
  int window_size = stoi(argv[3]);
  string input_filename = argv[4];
  string log_filename = argv[5];

  ofstream log_file(log_filename, ofstream::app);
  log_file.close();

  ifstream input_file(input_filename);

  if (!input_file.is_open()) {
    printf("Error: Cannot open input file\n");
    return 1;
  }

  // Create UDP socket
  int udp_sock = make_udp_socket();
  struct sockaddr_in addr;
  const char *hostname = receiver_ip.c_str();
  if (make_client_sockaddr(&addr, hostname, receiver_port) == -1) {
    cerr << "Error making client sockaddr" << endl;
  }

  // Send START message to initiate connection
  // START: type = 0; seqNum = random(); length = 0; checksum = ?

  // Wait for ACK of START

  // Send END message to end connection
  // END: type = 1; seqNum = same as START; length = 0; checksum = ?

  // Wait for ACK of END
}