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
    int a = 21;
    int b = 42;
    /* call the add function */
    int c = simpleAdditionTMR(a, b);

    if (63 == c) {
        /* The result is correct */
        return 0;
    } else {
        /* The result is not the expected */
        return 1;
    }
}
