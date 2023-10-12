#include <stdio.h>

int main() { 
    int a, b, c, i;                
    for (i = 0; i < 1000000; i++) { 
        if (i % 4)  a = 10;
        if (i % 7)  a = 11;
        a = 12;
    }
    return 0;
}
