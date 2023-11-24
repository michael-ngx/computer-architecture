#include <stdlib.h>

// q1.cfg: Cache has 64 sets, 4 blocks per set, 64 bytes per block
// Therefore, with next line prefetching, we each fetch current block (64 bytes) and next block (64 bytes)
// -> With 128-byte skips (2 blocks), miss rate must be high
// int main() {
//         // 1 MB array
//         char* arr = (char*)malloc(sizeof(char) * 1000000);

//         int i;
//         for (i = 0; i < 1000000; i += 128) {
//                 arr[i] = (i % 128);
//         }

//         return 0;
// }

// This version should have a very small miss rate (always prefetched)

int main() {
        // 1 MB array
        char* arr = (char*)malloc(sizeof(char) * 1000000);
        int i;
        for (i = 0; i < 1000000; i += 1) {
                arr[i] = (i % 128);
        }
        return 0;
}