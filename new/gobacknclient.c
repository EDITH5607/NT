#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/time.h>
#include<stdlib.h>

int main()  {
      // initialize the timeout
      struct timeval tv;
      tv.tv_sec = 2;
      tv.tv_usec = 0;

      // socket settings
      int s = socket(AF_INET, SOCK_STREAM,0);
      struct sockaddr_in addr = {AF_INET, htons(8080), inet_addr("127.0.0.1")};
      //settime out for the for recv
      setsockopt(s,SOL_SOCKET, SO_RCVTIMEO,(const char*)&tv, sizeof(tv));

      if(connect(s,(struct sockaddr*)&addr, sizeof(addr))<0) {
            printf("\nConnection Failed!!\n");
            return -1;
      }

      int base=0,next_seq = 0;
      int windowsize, total_frame,ack;

      printf("\nEnter the windowSize: ");
      scanf("%d", &windowsize);

      printf("\nEnter the total_frame: ");
      scanf("%d",&total_frame);

      while(base < total_frame) {
            // sending sequence upto window size and also chck didnt exceed totalframe
            while(next_seq < base + windowsize && next_seq < total_frame) {
                  printf("\n-> sending frame %d",next_seq);
                  send(s,&next_seq,sizeof(next_seq),0);
                  next_seq++;
            }

            //waiting for ack; recv zero after timeout
            if(recv(s,&ack,sizeof(ack),0)<0) {
                  printf("!!! Timeout Resending from Frame %d \n",base);
                  next_seq = base;
            }
            else {
                  // new value is send like ack is 2 and base is 1(last send)
                  if(ack>base) {
                        printf("<- Recieved ACK : %d (window slides)\n",ack);
                        base = ack;
                  }
            }
      }
      printf("\nAll frames send and acknoledge succesflly\n");
      close(s);
      return 0;

}