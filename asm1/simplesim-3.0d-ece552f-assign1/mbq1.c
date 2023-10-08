#include <stdio.h>

int main() {
    int a = 1;       // $5
    int b = 2;       // $6
    int c = 0;       // $7
    int count = 4114;     // $3
    int i;                 // $4

    for (i = 0; i < 1000000; i++) {        // $11 stores 55061
        a = b + c;          // addu $5,$6,$7
        b = a + c;          // addu $6,$5,$7
        c = a + b;          // addu $7,$5,$6
        count++;
        b = a + c;          // addu $6,$5,$7
        
        // // Prevent overflow
        if (a > 857808) a = a % count;
        if (b > 326498) b = b % count;
        if (c > 728974) c = c % count;
    }
    // Prevent optimized away
    printf("%lld %lld %lld", a, b, c);
    return 0;
}