
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

const int PORT = 12345;

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  const char *message = "Hello, UDP Receiver!";

  // Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return 1;
  }

  // Define server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;                     // IPv4
  server_addr.sin_port = htons(PORT);                   // Port
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

  // Send message to the receiver
  ssize_t sent_bytes =
      sendto(sockfd, message, strlen(message), 0,
             (const struct sockaddr *)&server_addr, sizeof(server_addr));

  if (sent_bytes < 0) {
    perror("Send failed");
    close(sockfd);
    return 1;
  }

  std::cout << "Message sent: " << message << std::endl;

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
//   if (argc != 6) {
//     printf("Usage: %s <receiver_ip> <receiver_port> <window_size> "
//            "<input_filename> <log_filename>\n",
//            argv[0]);
//     return 1;
//   }

//   string receiver_ip = argv[1];
//   int receiver_port = stoi(argv[2]);
//   int window_size = stoi(argv[3]);
//   string input_filename = argv[4];
//   string log_filename = argv[5];

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