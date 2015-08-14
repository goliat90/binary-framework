/*  Test code consisting of main calling a simple add function. */

/*  Divistion function test. */
int modulusTest(int a, int b, int c, int d) {
    int result = 0;

    result = a % b;
    result += b % c;
    result += c % d;
    result += d % a;

    /*  Return the result */
    return result;
}

/* Main sets two integer values. */
int main(int argc, char** argv) {
    /* values to be added */
    int a = argv[0][0] - '0';
    int b = argv[1][0] - '0';
    int c = argv[2][0] - '0';
    int d = argv[3][0] - '0';;

    /* call the add function */
    return modulusTest(a, b, c, d);
}
