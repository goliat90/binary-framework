/*  This test is intended to test the ability
    to modify the stack as intended. */

#include "testHeader.h"

/*  Four argument should end up in a0-a3 and
    one should end up on the stack. */
int stackAllocationTest(int a, int b, int c, int d, int e) {
    /*  Just add the numbers together
        and return the sum. */
    int res = a;
    res += b;
    res += c;
    res += d;
    res += e;

    return res;
}

