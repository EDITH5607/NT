#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define MAXFRAMES 10
#define WINDOWSIZE 4
#define PORT 54362
#define TIMEOUT_SEC 2

typedef struct {
  int type;  // 1=ACK, 0=NACK
  int ack;
  int seq;
  char data[1024];
} Frame;

// Timer structure for each frame
typedef struct {
  int seq;
  int acked;
  struct timeval send_time;
  int timer_started;
} FrameTimer;

int main() {
  int sock, next = 0, base = 0;
  struct sockaddr_in server_addr;
  socklen_t server_len;
  Frame frame;

  FrameTimer timers[MAXFRAMES];
  int acked[MAXFRAMES] = {0};

  struct timeval tv = {TIMEOUT_SEC, 0};

  // Create socket
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    printf("Client socket creation failed\n");
    exit(1);
  }

  // Setup server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_len = sizeof(server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Set timeout for recvfrom
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  // Initialize timers
  for (int i = 0; i < MAXFRAMES; i++) {
    timers[i].seq = i;
    timers[i].acked = 0;
    timers[i].timer_started = 0;
  }

  printf("[CLIENT] Selective Repeat Started\n");
  printf("[CLIENT] Total Frames: %d, Window Size: %d\n", MAXFRAMES, WINDOWSIZE);

  while (base < MAXFRAMES) {
    // ========== PHASE 1: SEND FRAMES IN WINDOW ==========
    while (next < base + WINDOWSIZE && next < MAXFRAMES) {
      if (!timers[next].acked) {
        frame.seq = next;
        frame.type = 0;
        printf("[CLIENT] Sending Frame %d\n", frame.seq);
        sendto(sock, &frame, sizeof(frame), 0, (struct sockaddr*)&server_addr,
               server_len);

        // Start timer for this frame
        gettimeofday(&timers[next].send_time, NULL);
        timers[next].timer_started = 1;
      }
      next++;
    }

    // ========== PHASE 2: CHECK FOR TIMEOUTS ==========
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    for (int i = base; i < next; i++) {
      if (!timers[i].acked && timers[i].timer_started) {
        long elapsed =
            (current_time.tv_sec - timers[i].send_time.tv_sec) * 1000 +
            (current_time.tv_usec - timers[i].send_time.tv_usec) / 1000;

        if (elapsed >= TIMEOUT_SEC * 1000) {
          //  SELECTIVE REPEAT: Resend ONLY this frame
          printf("[CLIENT]  TIMEOUT! Resending Frame %d only\n", i);
          frame.seq = i;
          sendto(sock, &frame, sizeof(frame), 0, (struct sockaddr*)&server_addr,
                 server_len);

          // Restart timer
          gettimeofday(&timers[i].send_time, NULL);
        }
      }
    }

    // ========== PHASE 3: RECEIVE ACKS/NACKS ==========
    Frame response;
    int r = recvfrom(sock, &response, sizeof(response), 0,
                     (struct sockaddr*)&server_addr, &server_len);

    if (r > 0) {
      if (response.type == 1)  // ACK received
      {
        int ack_num = response.ack;
        if (!timers[ack_num].acked) {
          timers[ack_num].acked = 1;
          acked[ack_num] = 1;
          printf("[CLIENT]  ACK %d received\n", ack_num);

          // Slide window - move base past all consecutive acked frames
          while (base < MAXFRAMES && timers[base].acked) {
            printf("[CLIENT] Window slid - Base now: %d\n", base + 1);
            base++;
          }
        }
      } else if (response.type == 0)  // NACK received
      {
        // SELECTIVE REPEAT: Resend ONLY the NACKed frame
        int nack_num = response.ack;
        printf(
            "[CLIENT]  NACK received for Frame %d - Resending immediately\n",
            nack_num);

        frame.seq = nack_num;
        sendto(sock, &frame, sizeof(frame), 0, (struct sockaddr*)&server_addr,
               server_len);

        // Restart timer
        gettimeofday(&timers[nack_num].send_time, NULL);
      }
    }

    usleep(10000);  // Small delay to prevent CPU spinning
  }

  printf("\n[CLIENT] All frames sent successfully!\n");
  close(sock);
  return 0;
}