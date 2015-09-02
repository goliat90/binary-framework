/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

/*  Divistion function test. */
int modulusTest(int a, int b, int c, int d) {
    int result = 0;

    result = a % b;
    result += b % c;
    result += c % d;
    result += d % a;

    /*  Return the result */
    return result;
}

