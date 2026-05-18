#include<stdio.h>

int main() {
      int n;
      printf("\nEnter the number of router: ");
      scanf("%d", &n);
      int cost[n][n],via[n][n],dist[n][n];

      //accepting values to costmatrix
      for (int i=0;i<n;i++) {
            for (int j = 0 ; j<n ; j++) {
                  printf("\nEnter %d source and %d destination cost:  ",i,j);
                  scanf("%d",&cost[i][j]);
                  if (cost[i][j] == -1) {
                        dist[i][j] = 999;
                        via[i][j] = -1;
                  }
                  else {
                        dist[i][j]=cost[i][j];
                        via[i][j]=j;
                  }
            }
      }

      // calculating distnce

      int flag;
      do {
            flag = 0;
            for(int i=0;i<n;i++){
                  for (int j=0;j<n;j++) {
                        for (int k=0;k<n;k++) {
                              if( dist[i][j] > dist[i][k]+dist[k][j]) {
                                    dist[i][j] = dist[i][k] + dist[k][j];
                                    via[i][j] = via[i][k];
                                    flag = 1;
                              }
                        }
                  }
            }

      }while(flag);


      for(int i = 0; i<n; i++) {
            printf("router %d\n", i);
            printf("\nDestination\tdistance\tnextroute\n");
            for(int j =0; j<n; j++) {
                  int d = dist[i][j] == 900 ? -1: dist[i][j];
                  printf("%d\t%d\t%d\n", j,d,via[i][j]);
            }
      }
      return 0;
}