#include "PacketHeader.h"
#include "helper.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
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

  // Testing info
  std::cout << "Number of packets: " << packets.size() << std::endl;
  std::cout << "Packet 1 length: " << packets[0].header.length << std::endl;
  std::cout << "Packet 1 data: " << packets[0].data << std::endl;

  std::cout << "Last packet length: "
            << packets[packets.size() - 1].header.length << std::endl;
  std::cout << "Last packet data: " << packets[packets.size() - 1].data
            << std::endl;

  // Socket setup
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to make UDP socket." << std::endl;
    return 1;
  }

  struct sockaddr_in server_addr;
  socklen_t addr_len = sizeof(server_addr);

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(receiver_port);
  server_addr.sin_addr.s_addr =
      inet_addr(receiver_ip); // Localhost 127.0.0.1 for testing

  // Initiate connection
  start_connection(sockfd, server_addr);

  // Send data packets until all have been received
  send_packet(packets[packets.size() - 1], sockfd, server_addr);

  // End connection
  end_connection(sockfd, server_addr);
  return 0;
}
