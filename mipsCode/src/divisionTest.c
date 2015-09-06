/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

/*  Divistion function test. */
int divisionTest(int a, int b, int c, int d) {
    int result = 0;

    /*  Divide the numbers. */
    result = a / b;
    result = result / c;
    result = result / d;


    /*  Return the result */
    return result;
}
