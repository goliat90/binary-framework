

//Include the users header.
#include "userFramework.hpp"


//constructor.
userFramework::userFramework(int argc, char** argv) {
    //initialize the framework with a binary.
    initialize(argc, argv); 
}


int main(int argc, char** argv) {

    userFramework* uT = new userFramework(argc, argv);

    uT->transformBinary();

    return 0;
}


/* The user defined decision function */
void userFramework::transformDecision(SgAsmMipsInstruction*) {
    /*   */
}


