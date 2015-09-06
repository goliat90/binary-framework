/*  Main file to the stack alloc test. */

#include "testHeader.h"
#include <stdio.h>


/*  Main function for calling the stack alloc test. */
int main(int argc, char** argv) {
    if (6 <= argc) {
        int a = argv[1][0] - '0';
        int b = argv[1][0] - '0';
        int c = argv[1][0] - '0';
        int d = argv[1][0] - '0';
        int e = argv[1][0] - '0';
        int res = 0;

        /*  call stack alloc function. */
        res = stackAllocationTest(a, b, c, d, e);

        /*  printout. */
        printf("a = %d\nb = %d\nc = %d\nd = %d\ne = %d\nres = %d\n",
            a, b, c, d, e, res);
    }
    return 0;
}

