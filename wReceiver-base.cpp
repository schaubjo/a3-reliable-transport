
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
}