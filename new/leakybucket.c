#include<stdio.h>
int main() {
      int bucketsize,n,outputrate;
      int bucketcontent = 0;
      printf("\nEnter number of packets: ");
      scanf("%d",&n);
      int packets[n];

      //accepting packet
      for (int i=0;i<n;i++) {
            scanf("%d",&packets[i]);
      }

      printf("\nEnter the bucketSize: ");
      scanf("%d",&bucketsize);

      printf("\nEnter outputrate: ");
      scanf("%d", &outputrate);


      printf("packet | p-size | bucketcontent | status | send | remaining\n");
      for(int i=0; i<n;i++) {
            int p_size = packets[i];
            char* status = "accepted";

            //checking overflow
            if(bucketsize >= bucketcontent+p_size) {
                  bucketcontent += p_size;
                  status = "accepted";
            }
            else {
                  status = "dropped";
            }
            //saving current bucket content
            int current_bucket_content = bucketcontent;

            // transmit at constant rate or leak at constant rate.
            int transmitted = (bucketcontent <= outputrate) ? bucketcontent:outputrate;
            bucketcontent -= transmitted;

            printf("%d | %d | %d | %s | %d | %d\n",i+1, p_size, current_bucket_content, status, transmitted, bucketcontent);
      }
      return 0;
}