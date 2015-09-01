/*  Test code consisting of main calling a simple add function. */

/* Simple addiction function that is transformed */
int logicalTMR(int a, int b) {
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

/* Main sets two integer values. */
int main(int argc, char** argv) {
    /* values to be added */
    int a = argv[0][0] - '0';
    int b = argv[1][0] - '0';
    int c;

    /* call the logical function */
    c = logicalTMR(a, b);
    
    return c;
}
