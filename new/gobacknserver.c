#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>

int main() {
      //server settings
      int s = socket(AF_INET, SOCK_STREAM,0);
      struct sockaddr_in addr = {AF_INET, htons(8080), inet_addr("127.0.0.1")};
      bind(s,(struct sockaddr*)&addr, sizeof(addr));
      listen(s,3);
      int client = accept(s,0,0);


      int expectedseq = 0, seq;
      while (1)
      {
            int length = recv(client, &seq, sizeof(seq), 0);
            if(length<=0) {
                  break;
            }

            //simulate frame drop 1 in 5 frame drop
            if(rand() % 5 ==0) {
                  printf("Recieved Frame %d but simulated LOSS(Discarded)\n",seq);
                  continue;
            }

            printf("Recived frame : %d",seq);

            if(seq == expectedseq) {
                  printf(" [ACCEPTED]\n");
                  expectedseq++;
            }
            else {
                  printf(" [Out of Order - Discarded]\n  ");
            }

            send(client, &expectedseq,sizeof(expectedseq),0);
            printf("Send ACK: %d\n", expectedseq);
      }
      close(client);
      close(s);
      return 0;
}