/*  Test code consisting of main calling a simple add function. */

/*  Divistion function test. */
int divisionmodTest(int a, int b) {
    int divresult = 0;
    int modresult = 0;
    int modresult2 = 0;

    a = a + 2 + 1;
    b = b + a + 1;

    /*  Divide the numbers and do mod on them. */
    divresult = a / b;
    modresult = a % b;
    modresult2 = b % a;

    /*  Return the result */
    return (divresult + modresult + modresult2);
}

/* Main sets two integer values. */
int main(int argc, char** argv) {
    /* values to be added */
    int a = argv[0][0] - '0';
    int b = argv[1][0] - '0';
    int c = argv[2][0] - '0';
    int d = argv[3][0] - '0';;

    /* call the add function */
    return divisionmodTest(a, b);
}
