

//Include the users header.
#include "userFramework.hpp"


//constructor.
userFramework::userFramework(int argc, char** argv) {
    //initialize the framework with a binary.
    initialize(argc, argv); 
}


int main(int argc, char** argv) {

    userFramework* uT = new userFramework(argc, argv);

    uT->traverseBinary();

    return 0;
}


//user written decision function.
void userFramework::transformDecision(SgAsmStatement*) {
    //std::cout << "user decision function" << std::endl;
}


