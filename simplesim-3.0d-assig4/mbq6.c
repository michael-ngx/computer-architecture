#include<stdlib.h>

// Matrix multiplication (stride prefetching should be good here (easy pattern)
// Create 2D array of size 1000 x 1000, initialize to random values
// Also, have an additional miss queue
int main () {
        int i; int j; int k;
        int epoch;
        int** A = (int**)malloc(sizeof(int*) * 1000);
        int** B = (int**)malloc(sizeof(int*) * 1000);
        int** C = (int**)malloc(sizeof(int*) * 1000);
        for (i = 0; i < 1000; i++) {
                A[i] = (int*)malloc(sizeof(int) * 1000);
                B[i] = (int*)malloc(sizeof(int) * 1000);
                C[i] = (int*)malloc(sizeof(int) * 1000);
        }
        // Initialize matrices
        for (i = 0; i < 1000; i++) {
                for (j = 0; j < 1000; j++) {
                        A[i][j] = i * 13 + j;
                        B[i][j] = j * 47 - i;
                        C[i][j] = 0;
                }
        }

        // Matrix multiplication
        for (i = 0; i < 1000; i++) {
                for (j = 0; j < 1000; j++) {
                        for (k = 0; k < 1000; k++) {
                                C[i][j] = A[i][k] * B[k][j];
                        }
                }
        }
}
