/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

#include <stdio.h>

/* Main sets two integer values. */
int main(int argc, char** argv) {

    if (5 <= argc) {
        /* values to be added */
        int a = argv[1][0] - '0';
        int b = argv[2][0] - '0';
        int c = argv[3][0] - '0';
        int d = argv[4][0] - '0';;
        int res = 0;

        /* call the add function */
        res = modulusTest(a, b, c, d);

        printf("res = %d\n", res);
    }
    return 0;
}
