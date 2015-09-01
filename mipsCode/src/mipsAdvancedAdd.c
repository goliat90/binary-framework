/*  Test code consisting of main calling a advanced add function.
    The purpose of this code is to generate code that uses accumulator functions. */

/* Simple addiction function that is transformed */
long long advancedAdditionTMR(int a, int b) {
    /* add the numbers together and return them */
    /*  Using long long to get 64 bit variables. */
    long long acc1 = (long long) 0;
    long long acc2 = (long long) 0;

    /*  loop variables. */
    int j;
    /*  Get a high upper limit on the loop. */
    int limit = a*b*100 + 1000;

    /*  Loop and perform some work. */
    for (j = 0; j < limit; j++) {
        /*  computation on acc, plus operation. */
        acc1 += (long long) 213*a;
        acc1 += (long long) 311*b;
    }

    return acc1;
}

/* Main sets two integer values. */
int main(int argc, char** argv) {
    /* values to be added */
    int a = argv[0][0] - '0';
    int b = argv[1][0] - '0';
    int c;

    /* call the add function */
    c = advancedAdditionTMR(a, b);
    
    return c;
}
