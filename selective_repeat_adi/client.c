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
  int sock;
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);

  int acked[MAXFRAMES] = {0};
  int base = 0;
  int next = 0;
  Frame frame;

  struct timeval tv = {2, 0};

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    printf("Socket creation failed\n");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  while (base < MAXFRAMES) {
    while (next < MAXFRAMES && next < base + WINDOWSIZE) {
      frame.seq = next;
      printf("Sending Frame %d\n", frame.seq);
      sendto(sock, &frame, sizeof(frame), 0, (struct sockaddr*)&addr, addr_len);
      next++;
    }

    int r = recvfrom(sock, &frame, sizeof(frame), 0, (struct sockaddr*)&addr,
                     &addr_len);

    if (r < 0) {
      printf("Received NACK retransmitting frames\n");
      for (int i = 0; i < MAXFRAMES; i++) {
        if (!acked[i]) {
          frame.seq = i;
          printf("Sending Frame %d\n", frame.seq);
          sendto(sock, &frame, sizeof(frame), 0, (struct sockaddr*)&addr,
                 addr_len);
        }
      }
      continue;
    }
    printf("ACK %d received\n", frame.ack);

    acked[frame.ack] = 1;
    while (acked[base] && base < MAXFRAMES) {
      base++;
    }
  }
  printf("All frames are sent\n");
  close(sock);
  return 0;
}