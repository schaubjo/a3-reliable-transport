#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

const int PORT = 12345;
const int BUFFER_SIZE = 1024;

int main() {
  int sockfd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];

  // Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return 1;
  }

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;         // IPv4
  server_addr.sin_port = htons(PORT);       // Port
  server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces

  // Bind the socket
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    close(sockfd);
    return 1;
  }

  std::cout << "Waiting for messages..." << std::endl;

  // Receive messages
  ssize_t received_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                    (struct sockaddr *)&client_addr, &addr_len);

  if (received_bytes < 0) {
    perror("Receive failed");
    close(sockfd);
    return 1;
  }

  buffer[received_bytes] = '\0'; // Null-terminate the received string
  std::cout << "Message received: " << buffer << std::endl;

  // Send ACK message back to the sender
  const char *ack_message = "ACK: Message received";
  ssize_t sent_bytes = sendto(sockfd, ack_message, strlen(ack_message), 0,
                              (const struct sockaddr *)&client_addr, addr_len);

  if (sent_bytes < 0) {
    perror("ACK send failed");
    close(sockfd);
    return 1;
  }

  std::cout << "ACK sent: " << ack_message << std::endl;

  // Close the socket
  close(sockfd);
  return 0;
}

// #include "PacketHeader.h"
// #include "crc32.h"
// #include "socket.h"
// #include <arpa/inet.h>
// #include <chrono>
// #include <cstdint>
// #include <cstdio>
// #include <cstring>
// #include <fstream>
// #include <iostream>
// #include <string>
// #include <sys/socket.h>
// #include <thread>
// #include <unistd.h>

// #define MAX_PACKET_SIZE 1472
// #define START 0
// #define END 1
// #define DATA 2
// #define ACK 3

// using namespace std;

// int main(int argc, char *argv[]) {
//   if (argc != 5) {
//     printf("Usage: %s <port-num> <window-size> <output-dir> <log>\n",
//     argv[0]); return 1;
//   }
//   int port_num = stoi(argv[1]);
//   int window_size = stoi(argv[2]);
//   string output_dir = argv[3];
//   string log = argv[4];

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
// }