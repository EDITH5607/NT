#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>

int checkprime(int value) {
  int flag = 1;
  for (int i = 2; i <= value / 2; i++) {
    if (value % i == 0) {
      flag = 0;
    }
  }
  return flag;
}

int main() {
      int s = socket(AF_INET, SOCK_DGRAM, 0);
      int value;
      struct sockaddr_in addr = {AF_INET, htons(8080), inet_addr("127.0.0.1")};
      socklen_t len = sizeof(addr);
      bind(s, (struct sockaddr*)&addr, len);

      printf("Server is online !!!\n");
      recvfrom(s, &value, sizeof(value), 0,(struct sockaddr*)&addr,&len);

      int flag = checkprime(value);
      if (flag) {
            printf("SERVER : IS PRIME\n");
      }
      else {
            printf("SERVER : NOT PRIME\n");
      }
      sendto(s, &flag, sizeof(flag), 0, (struct sockaddr*)&addr, sizeof(addr));
      close(s);
      return 0;
}