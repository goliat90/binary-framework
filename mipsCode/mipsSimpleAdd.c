/*  Test code consisting of main calling a simple add function. */

/* Simple addiction function that is transformed */
int simpleAdditionTMR(int a, int b) {
    /* add the numbers together and return them */
    int result = a + b;
    return result;
}

/* Main sets two integer values. */
int main(int argc, char** argv) {
    /* values to be added */
    int a = argc;
    int b = 4;
    /* call the add function */
    int c = simpleAdditionTMR(a, b);

    if (8 <= c) {
        /* The result is correct */
        return 0;
    } else {
        /* The result is not the expected */
        return 1;
    }
}
