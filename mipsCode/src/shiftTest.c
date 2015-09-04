/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

/* Simple addiction function that is transformed */
int shiftTest(int a, int b) {
    /* add the numbers together and return them */
    int result = 0;

    /* testing slt and checking several of the shift functions. */
    if (a < b) {
        result += a << b;
        result -= b >> a;
    }

    if (a < 10) {
        result += 2 * a;
        result -= b/2;
    }

    return result;
}

