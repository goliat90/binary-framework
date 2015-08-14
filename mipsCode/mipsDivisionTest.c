/*  Test code consisting of main calling a simple add function. */

/*  Divistion function test. */
int divisionTest(int a, int b, int c, int d) {
    int result = 0;
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    int result5 = 0;
    int result6 = 0;
    int sum = 0;

    /*  add up a and b. */

    /*  Divide the numbers. */
    result = a / b;
//    result = a / c;
//    result = a / d;
    result += result / c;
//    result = b / d;
    result += result / d;


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
    return divisionTest(a, b, c, d);
}
