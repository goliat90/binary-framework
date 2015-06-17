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
    int a = argv[0][0] - '0';
    int b = argv[1][0] - '0';
    /* call the add function */
    int c = simpleAdditionTMR(a, b);

    return c;
}