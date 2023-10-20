#include <stdio.h>

int main() {
    int result = 0;

    // Loop a significant number of times to stress-test the predictor.
    // The expected branch outcome would be:
    // T - (then inner outcomes) * (1000000 / 3) - N
    for (int i = 0; i < 1000000; i++) {
        // Nested loops can create more complex branch patterns.
        // The expected branch outcome is (inner outcomes)(for-if outcome pairs):
        // Part 1) i % 3 == 0: TT TN TN TT TN TN TT TN TN TT N
        // Part 2) i % 3 == 1: TN TN TT TN TN TT TN TN TT TN N
        // Part 3) i % 3 == 2: TN TT TN TN TT TN TN TT TN TN N
        for (int j = 0; j < 10; j++) {
            if ((i+j) % 3 == 0) {
                result += j;
            }
            result -= j;
        }
    }
    printf("Result: %d\n", result); // To avoid optimization out.
    return 0;
}