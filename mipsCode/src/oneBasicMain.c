/*  Test code consisting of main calling a simple add function. */

#include "testHeader.h"

#include <stdio.h>


/* Main sets two integer values. */
int main(int argc, char** argv) {
    int c = 0;

    if (argc >= 3) {
        printf("argc = %d\n", argc);
        /* values to be added */
        int a = argv[1][0] - '0';
        int b = argv[2][0] - '0';

        printf("Calling oneBasic.\n");

        /* call the add function */
        c = oneBasic(a, b);
        /*  Print to verify that the results is as expected. */
        printf("a = %d \nb = %d \na + b + (a * b) = %d\n", a, b, c);
    }
    
    return c;
}
