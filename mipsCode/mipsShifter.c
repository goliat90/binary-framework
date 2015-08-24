/*  Test code consisting of main calling a simple add function. */

/* Simple addiction function that is transformed */
int shifterTMR(int a, int b) {
    /* add the numbers together and return them */
    int result = 0;

    /* testing slt and checking several of the shift functions. */
    if (a < b) {
        result += a << b;
        result -= b >> a;
    }

    if (a < 10) {
        result += 2 * a;
        result -= b/2;
    }

    return result;
}

/* Main sets two integer values. */
int main(int argc, char** argv) {
    /* values to be added */
    int a = argv[0][0] - '0';
    int b = argv[1][0] - '0';
    int c;

    /* call the add function */
    if (argc > 3) {
        c = shifterTMR(a, b);
    } 
    
    return c;
}
