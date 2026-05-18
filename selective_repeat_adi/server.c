#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PORT 54362
#define MAXFRAMES 10
#define WINDOWSIZE 4

typedef struct {
  int type;  // 1=ACK, 0=NACK
  int ack;
  int seq;
  char data[1024];
} Frame;

int main() {
  int sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  Frame frame, response;
  int received[MAXFRAMES] = {0};
  int base = 0;

  // Create UDP socket
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    printf("Server socket creation failed\n");
    exit(1);
  }

  // Setup server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket
  if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    printf("Binding failed\n");
    close(sock);
    exit(1);
  }

  printf("========================================\n");
  printf("SELECTIVE REPEAT SERVER STARTED\n");
  printf("Port: %d | Max Frames: %d | Window Size: %d\n", PORT, MAXFRAMES,
         WINDOWSIZE);
  printf("========================================\n\n");

  srand(time(NULL));

  while (base < MAXFRAMES) {
    // Receive frame from client
    int n = recvfrom(sock, &frame, sizeof(frame), 0,
                     (struct sockaddr*)&client_addr, &client_len);

    if (n <= 0) {
      continue;
    }

    // Simulate packet loss (30% chance)
    if (rand() % 10 < 3) {
      printf("[LOSS]  Frame %d DROPPED (simulated loss)\n", frame.seq);
      continue;
    }

    printf("[RECV] Received Frame %d\n", frame.seq);

    // Check if frame sequence number is within receive window
    if (frame.seq >= base && frame.seq < base + WINDOWSIZE) {
      // Accept the frame
      if (!received[frame.seq]) {
        received[frame.seq] = 1;
        printf("[BUFFER] Frame %d stored in buffer\n", frame.seq);
      }

      // Send ACK for this frame (Selective Repeat ACKs every frame)
      response.type = 1;  // ACK
      response.ack = frame.seq;

      sendto(sock, &response, sizeof(response), 0,
             (struct sockaddr*)&client_addr, client_len);
      printf("[ACK]  Sent ACK %d\n", frame.seq);

      // Try to deliver consecutive frames starting from base
      while (base < MAXFRAMES && received[base]) {
        printf("[DELIVER]  Delivered Frame %d to application\n", base);
        base++;
      }

      printf("[WINDOW] Current receive window base: %d\n\n", base);
    } else if (frame.seq < base) {
      // Duplicate frame - resend ACK
      response.type = 1;  // ACK
      response.ack = frame.seq;

      sendto(sock, &response, sizeof(response), 0,
             (struct sockaddr*)&client_addr, client_len);
      printf("[DUP]  Duplicate Frame %d, ACK resent\n\n", frame.seq);
    } else if (frame.seq >= base + WINDOWSIZE) {
      // Frame outside window - ignore (send NACK for missing frames?)
      printf("[OUT]  Frame %d outside window [%d-%d], ignoring\n\n", frame.seq,
             base, base + WINDOWSIZE - 1);

      // Optional: Send NACK for base frame to help sender
      if (!received[base]) {
        response.type = 0;  // NACK
        response.ack = base;

        sendto(sock, &response, sizeof(response), 0,
               (struct sockaddr*)&client_addr, client_len);
        printf("[NACK]  Sent NACK for missing Frame %d\n\n", base);
      }
    }
  }

  printf("\n========================================\n");
  printf(" ALL FRAMES RECEIVED SUCCESSFULLY!\n");
  printf("========================================\n");

  close(sock);
  return 0;
}