/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

/* Main sets two integer values. */
int main(int argc, char** argv) {
    /* values to be added */
    int a = argv[1][0] - '0';
    int b = argv[2][0] - '0';

    /* call the add function */
    return divisionModulusTest(a, b);
}
