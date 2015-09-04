/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

#include <stdio.h>

/* Main sets two integer values. */
int main(int argc, char** argv) {
    int c = 0;
    
    if (3 <= argc) {
        /* values to be added */
        int a = argv[1][0] - '0';
        int b = argv[2][0] - '0';

        /* call the logical function */
        c = logicalTest(a, b);

        printf("a = %d\nb = %d\nc = %d\n", a, b, c);
    }
    return 0;
}
