#include <stdlib.h>

// Stride prefetching should perform well on nested loops
// int main () {
//         int a = 74;
//         int b = 29;
//         int i; int j;

//         char* arr = (char*)malloc(sizeof(char) * 1000000);
//         for (i = 0; i < 100000; i++) {
//                 for (j = 0; j < 800; j += a) {
//                         arr[j] = (ju % 128);

//                         if (a == b) a = b/2;
//                         else a = b;
//                 }
//         }
// }

// Intentionally create a loop that will be harder to predict on a single loop
// Apparently, this gives a high miss rate
int main () {
        int a = 74;
        int b = 29;
        int i;

        char* arr = (char*)malloc(sizeof(char) * 1000000);
        for (i = 0; i < 1000000; i += a) {
                arr[i] = (i % 128);

                if (a == b) a = b/2;
                else a = b;
        } 
}

