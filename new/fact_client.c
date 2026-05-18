#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
int main() {
      long long  fact;
      int value;
      int s = socket(AF_INET, SOCK_STREAM, 0);
      struct sockaddr_in a = {AF_INET, htons(8080),inet_addr("127.0.0.1")};
      if(connect(s,(struct sockaddr*)&a, sizeof(a))<0)
            return 1;
      printf("client started!!\n\n");
      printf("Connected to server!!\n");
      printf("Enter the number: ");
      scanf("%d",&value);

      send(s,&value, sizeof(value),0);
      recv(s,&fact, sizeof(fact), 0);
      printf("Server : factorial -> %lld\n",fact);
      close(s);
      return 0;
}