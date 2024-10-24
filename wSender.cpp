
#include <cstdio>
#include <string>
#include <fstream>
#include "PacketHeader.h"
#include "crc32.h"

#define MAX_PACKET_SIZE 1472
#define START 0
#define END 1
#define DATA 2
#define ACK 3

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 6) {
    printf(
        "Usage: %s <receiver_ip> <receiver_port> <window_size> <input_filename> <log_filename>\n",
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

  
}