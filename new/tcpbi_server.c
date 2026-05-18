#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

int main() {
      int n;
      int s = socket(AF_INET, SOCK_STREAM, 0);
      struct sockaddr_in addr = {AF_INET, htons(8080)};
      inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
      char b[1024];

      bind(s, (struct sockaddr *)&addr,sizeof(addr));
      listen(s, 5);
      printf("Server is live!!\n");
      n = accept(s,0,0);
      while (1)
      {
        memset(b, 0, sizeof(b));
        if(read(n,b,sizeof(b))<= 0 ||!strncmp(b,"exit",4))
            break;
        printf("client : %s\nServer : ",b );
        fgets(b,sizeof(b), stdin);
        send(n,b,strlen(b),0);
        if (!strncmp(b,"exit",4))
            break;
      }
      close(n);
      close(s);
      return 0;
      
}