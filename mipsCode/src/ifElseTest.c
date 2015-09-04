/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

/* Simple function that results in a single basic block. */
int ifElseTest(int a, int b) {
    /* add the numbers together and return them */
    int result = 0;
    /*  Add together. */
    if (a < b) {
        result += a;
        result += b;
    } else {
        result += a * b;
    }
    return result;
}

