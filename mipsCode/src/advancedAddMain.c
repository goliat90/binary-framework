/*  Test code consisting of main calling a advanced add function.
    The purpose of this code is to generate code that uses accumulator functions. */

#include "testHeader.h"

#include <stdio.h>

/* Main sets two integer values. */
int main(int argc, char** argv) {
    if (3 >= argc) {
        /* values to be added */
        int a = argv[1][0] - '0';
        int b = argv[2][0] - '0';
        long long c = 0;

        /* call the add function */
        c = advancedAdd(a, b);

        printf("a = %d\nb = %d\nc = %lld", a, b, c);
    }
    
    return 0;
}
