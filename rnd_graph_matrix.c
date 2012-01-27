#include <stdio.h>
#include <stdlib.h>

int main (void) 
{
	int N = 16;
	int G[N][N];
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			G[i][j] = rand() %2;
			printf("%d",G[i][j]);
		}
		printf("\n");
	}


	return(0);
}
