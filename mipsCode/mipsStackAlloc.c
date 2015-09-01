/*  This test is intended to test the ability
    to modify the stack as intended. */


/*  Four argument should end up in a0-a3 and
    one should end up on the stack. */
int stackAlloc(int a, int b, int c, int d, int e) {
    /*  Just add the numbers together
        and return the sum. */
    return (a + b + c + d + e);

}

