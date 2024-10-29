#include "PacketHeader.h"
#include "helper.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[]) {
  // Parse command line arguments
  if (argc != 6) {
    printf("Usage: %s <receiver_ip> <receiver_port> <window_size> "
           "<input_filename> <log_filename>\n",
           argv[0]);
    return 1;
  }

  const char *receiver_ip = argv[1];
  const int receiver_port = std::stoi(argv[2]);
  const int window_size = std::stoi(argv[3]);
  const std::string input_filename = argv[4];
  const std::string log_filename = argv[5];

  // Transform input file into vector of packets
  std::vector<Packet> packets = packet_data_init(input_filename);

  // Socket setup
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to make UDP socket." << std::endl;
    return 1;
  }

  struct sockaddr_in server_addr;

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(receiver_port);
  server_addr.sin_addr.s_addr =
      inet_addr(receiver_ip); // Localhost 127.0.0.1 for testing

  // Generate random initial seq num for START
  int start_seq_num = generate_start_seq_num();

  // Initiate connection
  start_connection(server_addr, sockfd, start_seq_num);

  // Send data packets until all have been received
  send_packet(packets[0], server_addr, sockfd);

  // End connection
  end_connection(server_addr, sockfd, start_seq_num);
  return 0;
}
