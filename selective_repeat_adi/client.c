#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT 54362
#define MAXFRAMES 10
#define WINDOW_SIZE 4
#define TIMEOUT_SEC 2
#define BUFFER_SIZE 1024

typedef struct {
  int seq;
  int ack;
  int type;  // 1=ACK, 0=NACK
  char data[BUFFER_SIZE];
} Frame;

typedef struct {
  int seq;
  int acked;
  int timer_started;
  struct timeval send_time;
} TimerInfo;

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  socklen_t addr_len = sizeof(server_addr);

  Frame frames[MAXFRAMES];
  TimerInfo timers[MAXFRAMES];
  int base = 0;
  int next_seq = 0;
  int ack_count = 0;

  // Create UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    printf("Client socket creation failed\n");
    exit(1);
  }

  // Setup server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Set socket timeout
  struct timeval tv;
  tv.tv_sec = TIMEOUT_SEC;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  // Initialize frames with dummy data
  for (int i = 0; i < MAXFRAMES; i++) {
    frames[i].seq = i;
    frames[i].type = 0;
    sprintf(frames[i].data, "This is frame number %d", i);

    timers[i].seq = i;
    timers[i].acked = 0;
    timers[i].timer_started = 0;
  }

  printf("[CLIENT] Selective Repeat Client Started\n");
  printf("[CLIENT] Total Frames: %d, Window Size: %d\n", MAXFRAMES,
         WINDOW_SIZE);
  printf("[CLIENT] Timeout: %d seconds\n\n", TIMEOUT_SEC);

  while (ack_count < MAXFRAMES) {
    // Send frames in current window
    while (next_seq < base + WINDOW_SIZE && next_seq < MAXFRAMES) {
      if (!timers[next_seq].acked) {
        // Send frame
        sendto(sockfd, &frames[next_seq], sizeof(Frame), 0,
               (struct sockaddr*)&server_addr, addr_len);
        printf("[CLIENT] Sent Frame %d\n", next_seq);

        // Start timer for this frame
        gettimeofday(&timers[next_seq].send_time, NULL);
        timers[next_seq].timer_started = 1;
      }
      next_seq++;
    }

    // Check for timeouts
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    for (int i = base; i < next_seq; i++) {
      if (!timers[i].acked && timers[i].timer_started) {
        long elapsed =
            (current_time.tv_sec - timers[i].send_time.tv_sec) * 1000 +
            (current_time.tv_usec - timers[i].send_time.tv_usec) / 1000;

        if (elapsed >= TIMEOUT_SEC * 1000) {
          // Timeout - resend ONLY this frame (Selective Repeat!)
          printf("[CLIENT] TIMEOUT! Resending Frame %d only\n", i);
          sendto(sockfd, &frames[i], sizeof(Frame), 0,
                 (struct sockaddr*)&server_addr, addr_len);

          // Restart timer
          gettimeofday(&timers[i].send_time, NULL);
        }
      }
    }

    // Receive ACKs/NACKs
    Frame response;
    int n = recvfrom(sockfd, &response, sizeof(response), 0,
                     (struct sockaddr*)&server_addr, &addr_len);

    if (n > 0) {
      if (response.type == 1) {  // ACK received
        int ack_num = response.ack;
        if (!timers[ack_num].acked) {
          timers[ack_num].acked = 1;
          ack_count++;
          printf("[CLIENT] Received ACK %d (Total ACKs: %d/%d)\n", ack_num,
                 ack_count, MAXFRAMES);

          // Slide window
          while (base < MAXFRAMES && timers[base].acked) {
            printf("[CLIENT] Window slid - Base now: %d\n", base + 1);
            base++;
          }
        }
      } else if (response.type == 0) {  // NACK received
        int nack_num = response.ack;
        printf("[CLIENT] Received NACK for Frame %d - Resending immediately\n",
               nack_num);

        // Selective Repeat: Resend ONLY the NACKed frame
        sendto(sockfd, &frames[nack_num], sizeof(Frame), 0,
               (struct sockaddr*)&server_addr, addr_len);

        // Restart timer
        gettimeofday(&timers[nack_num].send_time, NULL);
      }
    }

    // Small delay to prevent CPU spinning
    usleep(10000);
  }

  printf("\n[CLIENT] All %d frames sent and acknowledged successfully!\n",
         MAXFRAMES);
  close(sockfd);
  return 0;
}