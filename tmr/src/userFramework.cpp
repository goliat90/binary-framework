

//Include the users header.
#include "userFramework.hpp"


//constructor.
userFramework::userFramework(int argc, char** argv) {
    //initialize the framework with a binary.
    initialize(argc, argv); 
}


int main(int argc, char** argv) {

    userFramework* uT = new userFramework(argc, argv);

    /* set which function is to be transformed */
    //functionSelect(string);

    uT->transformBinary();

    return 0;
}


/* The user defined decision function */
void userFramework::transformDecision(SgAsmMipsInstruction* inst) {
    /* Decode the instruction to get the information  */
    instructionStruct currentInst = decodeInstruction(inst);
    /* Depending on the kind of instruction do appropriate transformation
        consider more functions. */
    switch(currentInst.kind) {
        case mips_add: {
            /* Add two new add instructions using the original input operands */
            //TODO need a good way to build an instruction.
            //probably a reverse of decode instruction
            /* another add to combine the result of two inserted adds,
                one more to combine the result of the original instruction
                result and the combined result. */

            /* use one of the generated register and set it to three
                so it can be used in the division  */

            /* division instruction with the total sum of the adds
                that is divided by three. Giving us the average value.  */

            /* Move the result from the special register */
        }
        case mips_addi: {

        }
    }
}


