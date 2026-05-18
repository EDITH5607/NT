#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>

int main() {
      int s = socket(AF_INET, SOCK_DGRAM, 0);
      int value, flag;
      // initaialize struct...
      struct sockaddr_in addr = {AF_INET, htons(8080), inet_addr("127.0.0.1")};
      socklen_t len = sizeof(addr);

      bind(s, (struct sockaddr*)&addr, sizeof(addr));
      printf("Client started !!\n");

      //accept value from the user...
      printf("CLIENT : Enter value -> ");
      scanf("%d",&value);

      // send value to server for computatin
      sendto(s,&value, sizeof(value),0,(struct sockaddr*)&addr, len);

      // recieve value from server
      recvfrom(s,&flag, sizeof(flag), 0,(struct sockaddr*)&addr, &len);

      if (flag) {
            printf("SERVER : Number is Prime");
      }
      else {
            printf("SERVER : Number is not prime!!");
      }
      return 0;
}