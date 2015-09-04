/*  Test code consisting of main calling a simple add function. */

/*  Divistion function test. */
int divisionModulusTest(int a, int b) {
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
