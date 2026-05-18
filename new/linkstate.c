#include<stdio.h>
int main() {
      int n,src,min,u,v;
      printf("Enter the number of routers : ");
      scanf("%d", &n);
      int cost[n][n], parent[n], dist[n];
      int visited[n];
      for (int i=0;i<n;i++) {
            visited[i] = 0;
            for(int j=0;j<n;j++) {
                  printf("\nEnter the cost %d -> %d : ",i,j);
                  scanf("%d",&cost[i][j]);
                  if (cost[i][j] == -1) {
                        cost[i][j] = 999;
                  }
            }
      }

      printf("\nEnter source: ");
      scanf("%d",&src);
      for (int i=0;i<n;i++) {
            dist[i] = cost[src][i];
            parent[i] = src;
      }
      visited[src] = 1;
      dist[src] = 0;
      int count = 1;
      while(count<n-1) {
            min = 999;
            //checking each node from src -> i
            for(int i=0;i<n;i++) {
                  // check the min distance from src which is not visited yet
                  if (dist[i]<min && !visited[i]) {
                        min = dist[i];
                        u = i;
                        visited[u] = 1;

                  }
            }
            for (v =0;v<n;v++) {
                  if(!visited[v] && dist[v] > min + cost[u][v]) {
                        dist[v] = min + cost[u][v];
                        parent[v] = u;
                  }
            }
            count++;
      }
      printf("\nRouter\tDistance\tparent\n");
      for(int i=0;i<n;i++) {
            printf("%d\t%d\t%d\n", i,dist[i], parent[i]);
      }

      return 0;
}