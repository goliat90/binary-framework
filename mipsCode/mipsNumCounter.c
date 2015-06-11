//test code takes ascii numbers, convert them and add them together.
//the sum of the numbers plus argc is returned.
int main(int argc, char **argv)
{
    //Declarations
    int acc = 0;
    int i = 0;

    //go through argv and convert and check the numbers to be valid.
    for (i = 1; i < argc; i++) {
        //Get the number and adjust it.
        int charNum = argv[i][0];
        charNum -= '0';
        //check if it is a number, if it is add it otherwise ignore.
        if (charNum >= 0 && charNum <= 9) {
            acc += charNum;
        }
    }
    //return the sum.
    return (acc+argc);
}

