#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAXFRAMES 10
#define WINDOWSIZE 4
#define PORT 53621

typedef struct {
  int seq, ack;
} Frame;
int main() {
  int received[MAXFRAMES] = {0};
  int sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);
  int base = 0;
  Frame frame, ack_frame;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    printf("Socket creation failed\n");
    close(sock);
    exit(1);
  }
  printf("Server socket creation success\n");

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    printf("Socket creation failed\n");
    close(sock);
    exit(1);
  }
  printf("Selective repeat started\n");
  srand(time(NULL));

  while (base < MAXFRAMES) {
    int n = recvfrom(sock, &frame, sizeof(frame), 0,
                     (struct sockaddr*)&client_addr, &client_len);
    if (n <= 0) {
      continue;
    }

    if (rand() % 10 < 3) {
      printf("[PACKET LOSS] dropped frame %d\n", frame.seq);
      continue;
    }
    printf("received frame %d\n", frame.seq);

    if (frame.seq >= base && frame.seq < base + WINDOWSIZE) {
      if (!received[frame.seq]) {
        received[frame.seq] = 1;
        printf("Frame %d buffered\n", frame.seq);
      }

      ack_frame.ack = frame.seq;
      sendto(sock, &ack_frame, sizeof(ack_frame), 0,
             (struct sockaddr*)&client_addr, client_len);
      printf("ACK %d sent\n", frame.seq);

      while (base < MAXFRAMES && received[base]) {
        printf("Delivered Frame %d\n", base);
        base++;
      }
    } else {
      ack_frame.ack = frame.seq;
      sendto(sock, &ack_frame, sizeof(ack_frame), 0,
             (struct sockaddr*)&client_addr, client_len);
      printf("Duplicate frame %d ACK resent\n", frame.seq);
    }
  }
  printf("All frame received successfully\n");
  close(sock);
  return 0;
}