/*  Test code consisting of main calling a simple add function. */

/* Simple function that results in a single basic block. */
int oneBasic(int a, int b) {
    /* add the numbers together and return them */
    int result = 0;
    /*  Add together. */
    result += a;
    result += b;
    result += a * b;

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
        c = oneBasic(a, b);
    } 
    
    return c;
}
