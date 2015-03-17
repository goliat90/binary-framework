

/* small testfile for compiling and testing the framework */

#include "binaryRewriter.hpp"


int main(int argc, char **argv) {
    //get framework object.
    BinaryRewriter rewriter(argc, argv);
    
    //initalize a traversal of the binary, no changes done.
    rewriter.traverseBinary();

    //print traversal information.
    rewriter.printInformation();

    return 0;
}

