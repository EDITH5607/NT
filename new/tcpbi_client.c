#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
int main() {
      int s = socket(AF_INET, SOCK_STREAM,0);
      struct sockaddr_in addr = {AF_INET, htons(8080)};
      inet_pton(s,"127.0.0.1", &addr.sin_addr);
      char b[1024];
      if(connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
            return 1;
      printf("Connected!! Type 'exit' to stop\n");
      while (1)
      {
            printf("Client : ");
            fgets(b,1024,stdin);
            send(s, b, strlen(b),0);
            if (!strncmp(b,"exit",4)) {
                  break;
            }
            memset(b,0,1024);
            if(read(s,b,1024) <= 0 || !strncmp(b,"exit",4)) {
                  break;
            }
            printf("server : %s",b);
      }
      close(s);
      return 0;
}