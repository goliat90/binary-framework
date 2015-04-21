

/* small testfile for compiling and testing the framework */

#include "binaryRewriter.hpp"


int main(int argc, char **argv) {
    //get framework object.
    BinaryRewriter rewriter(argc, argv);

//    SgAsmDirectRegisterExpression reg = generateSymbolicRegister();
//    
//    if (isSymbolicRegister(&reg))
//        std::cout << "reg: " << findSymbolicRegister(&reg)  << " present" << std::endl;

    

    //initalize a traversal of the binary, no changes done.
    rewriter.transformBinary();

    //print traversal information.
    rewriter.printInformation();

    return 0;
}

