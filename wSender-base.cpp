#include "PacketHeader.h"
#include "socket.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>

#define MAX_PACKET_SIZE 1472
#define START 0
#define END 1
#define DATA 2
#define ACK 3
const int RETRANSMISSION_TIMER = 500; // ms

int main(int argc, char *argv[]) {
  if (argc != 6) {
    printf("Usage: %s <receiver_ip> <receiver_port> <window_size> "
           "<input_filename> <log_filename>\n",
           argv[0]);
    return 1;
  }

  const char *receiver_ip = argv[1];
  int receiver_port = std::stoi(argv[2]);
  int window_size = std::stoi(argv[3]);
  std::string input_filename = argv[4];
  std::string log_filename = argv[5];

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to make UDP socket." << std::endl;
    return 1;
  }

  struct sockaddr_in server_addr;
  const char *message = "Hello, new UDP Receiver!";
  char ack_buffer[1024];
  socklen_t addr_len = sizeof(server_addr);

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;            // IPv4
  server_addr.sin_port = htons(receiver_port); // Port
  server_addr.sin_addr.s_addr =
      inet_addr(receiver_ip); // Localhost 127.0.0.1 for testing

  // Send START packet to initiate connection
  while (true) {
    PacketHeader start_packet;
    start_packet.type = htonl(START);
    start_packet.length = htonl(0);
    start_packet.seqNum = htonl(4);    // TODO: change to rand()
    start_packet.checksum = htonl(31); // TODO: add crc
    std::cout << "Sending START" << std::endl;
    if (send_packet_header(start_packet, sockfd, server_addr) < 0) {
      return 1;
    }

    // Resend START if not acknowledged within 500 ms
    auto start_time = std::chrono::steady_clock::now();
    bool acked = false;
    while (true) {
      // Check for ACK
      PacketHeader ack_header;
      if (receive_packet_header(ack_header, sockfd, server_addr) &&
          ntohl(ack_header.type) == ACK &&
          ntohl(ack_header.seqNum) == htonl(start_packet.seqNum)) {
        std::cout << "ACK received for START" << std::endl;
        acked = true;
        break;
      }

      // Check if need to retransmit
      auto current_time = std::chrono::steady_clock::now();
      auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            current_time - start_time)
                            .count();
      if (elapsed_ms >= RETRANSMISSION_TIMER) {
        std::cout << "Retransmitting packet..." << std::endl;
        break; // Exit the inner loop to retransmit the packet
      }
    }

    if (acked) {
      break;
      std::cout << "Breaking out of START loop" << std::endl;
    }
  }

  // if (ntohl(ack_header.type) == ACK &&
  //     ntohl(ack_header.seqNum) == htonl(start_packet.seqNum)) {
  //   std::cout << "ACK for START received" << std::endl;
  // }
  // // Send message to the receiver
  // ssize_t sent_bytes =
  //     sendto(sockfd, message, strlen(message), 0,
  //            (const struct sockaddr *)&server_addr, sizeof(server_addr));

  // if (sent_bytes < 0) {
  //   perror("Send failed");
  //   close(sockfd);
  //   return 1;
  // }

  // std::cout << "Message sent: " << message << std::endl;

  // // Wait for ACK from the receiver
  // ssize_t ack_received = recvfrom(sockfd, ack_buffer, sizeof(ack_buffer) - 1,
  // 0,
  //                                 (struct sockaddr *)&server_addr,
  //                                 &addr_len);

  // if (ack_received < 0) {
  //   perror("ACK receive failed");
  //   close(sockfd);
  //   return 1;
  // }

  // ack_buffer[ack_received] = '\0'; // Null-terminate the ACK string
  // std::cout << "ACK received: " << ack_buffer << std::endl;

  // Close the socket
  close(sockfd);
  return 0;
}

// #define MAX_PACKET_SIZE 1472
// #define START 0
// #define END 1
// #define DATA 2
// #define ACK 3

// using namespace std;

// int main(int argc, char *argv[]) {
// if (argc != 6) {
//   printf("Usage: %s <receiver_ip> <receiver_port> <window_size> "
//          "<input_filename> <log_filename>\n",
//          argv[0]);
//   return 1;
// }

// string receiver_ip = argv[1];
// int receiver_port = stoi(argv[2]);
// int window_size = stoi(argv[3]);
// string input_filename = argv[4];
// string log_filename = argv[5];

//   // // ofstream log_file(log_filename, ofstream::app);
//   // // log_file.close();

//   // // ifstream input_file(input_filename);

//   // // if (!input_file.is_open()) {
//   // //   printf("Error: Cannot open input file\n");
//   // //   return 1;
//   // // }

//   // // Create UDP socket
//   // int client_sock = make_udp_socket();
//   // struct sockaddr_in addr;
//   // const char *hostname = receiver_ip.c_str();
//   // if (make_client_sockaddr(&addr, hostname, receiver_port) == -1) {
//   //   cerr << "Error making client sockaddr" << endl;
//   // }

//   // // Send START message to initiate connection
//   // // START: type = 0; seqNum = random(); length = 0; checksum = ?
//   // PacketHeader header;
//   // header.type = htonl(0);
//   // header.seqNum = htonl(1);   // TODO: make rand()
//   // header.checksum = htonl(2); // TODO: change
//   // header.length = htonl(0);   // Set to 0 for START
//   // const char *data = "";
//   // if (send_packet(client_sock, header, data, addr) < 0) {
//   //   return 1;
//   // }
//   // std::this_thread::sleep_for(std::chrono::seconds(100));
//   // // Wait for ACK of START

//   // // Do main logic of sending file data

//   // // Send END message to end connection
//   // // END: type = 1; seqNum = same as START; length = 0; checksum = ?

//   // // Wait for ACK of END
// }