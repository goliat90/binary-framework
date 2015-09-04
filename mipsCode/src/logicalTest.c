/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

/* Simple addiction function that is transformed */
int logicalTest(int a, int b) {
    /* add the numbers together and return them */
    int result = 0;


    /*  Testing logical operations. */
    if (a < b) {
        result = a & b;
        result &= a & 7;
    }
    
    if (b > 8) {
        result |= a | b;
        result |= b | 8;
    }

    return result;
}
