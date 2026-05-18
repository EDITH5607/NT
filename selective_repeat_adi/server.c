#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 54362
#define MAXFRAMES 10
#define BUFFER_SIZE 1024

typedef struct {
  int seq;
  int ack;
  int type;  // 1=ACK, 0=NACK
  char data[BUFFER_SIZE];
} Frame;

int main() {
  int sockfd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);

  Frame received_frames[MAXFRAMES];
  int expected_seq = 0;

  // Create UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    printf("Server socket creation failed\n");
    exit(1);
  }

  // Setup server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket
  if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    printf("Bind failed\n");
    exit(1);
  }

  printf("[SERVER] Selective Repeat Server started on port %d\n", PORT);
  printf("[SERVER] Waiting for frames...\n\n");

  // Initialize received frames buffer
  for (int i = 0; i < MAXFRAMES; i++) {
    received_frames[i].seq = -1;
  }

  while (expected_seq < MAXFRAMES) {
    Frame frame;
    int n = recvfrom(sockfd, &frame, sizeof(frame), 0,
                     (struct sockaddr*)&client_addr, &addr_len);

    if (n > 0) {
      printf("[SERVER] Received Frame %d\n", frame.seq);

      // Check if this is the expected frame
      if (frame.seq == expected_seq) {
        // Expected frame received
        received_frames[frame.seq] = frame;
        frame.type = 1;  // ACK
        frame.ack = frame.seq;
        sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr*)&client_addr,
               addr_len);
        printf("[SERVER] Sent ACK %d\n", frame.seq);

        expected_seq++;

        // Check if any buffered frames can be delivered
        while (expected_seq < MAXFRAMES &&
               received_frames[expected_seq].seq != -1) {
          printf("[SERVER] Delivered buffered Frame %d\n", expected_seq);
          expected_seq++;
        }
      } else if (frame.seq > expected_seq) {
        // Out-of-order frame - buffer it
        received_frames[frame.seq] = frame;
        frame.type = 1;  // Still send ACK
        frame.ack = frame.seq;
        sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr*)&client_addr,
               addr_len);
        printf("[SERVER] Out-of-order Frame %d - Buffered, Sent ACK %d\n",
               frame.seq, frame.seq);
      } else {
        // Duplicate frame - resend ACK
        frame.type = 1;
        frame.ack = frame.seq;
        sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr*)&client_addr,
               addr_len);
        printf("[SERVER] Duplicate Frame %d - Resent ACK\n", frame.seq);
      }
    }
  }

  printf("\n[SERVER] All frames received successfully!\n");
  close(sockfd);
  return 0;
}