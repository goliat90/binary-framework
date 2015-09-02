/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

/* Simple addiction function that is transformed */
int simpleAdd(int a, int b) {
    /* add the numbers together and return them */
    int result = 0;
    int j;

    for (j = 0; j < a; j++) {
        result++;
        if (result < b) {
            result++;
        }
    }

    if (a < b) {
        result += a * b;
    }
    return result;
}
