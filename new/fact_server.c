#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>

long long fact(int n) {
      long long value = 1;
      for(int i=1;i<=n;i++) {
            value *=i;
      }
      return value;
}

int main() {
      int s = socket(AF_INET,SOCK_STREAM, 0);
      int n;
      int value;
      struct sockaddr_in addr = {AF_INET, htons(8080), inet_addr("127.0.0.1")};
      bind(s, (struct sockaddr*)&addr, sizeof(addr));
      listen(s,3);
      printf("server Listens to 8080....\n");
      n = accept(s,0,0);
      recv(n,&value,sizeof(value), 0);
      long long  factorial = fact(value);
      printf("server : %lld\n", factorial);
      send(n,&factorial, sizeof(factorial),0);
      close(n);
      printf("server : Connection Closed!!\n");
      return 0;
}