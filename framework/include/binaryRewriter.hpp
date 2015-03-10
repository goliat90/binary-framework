/* Header file for binaryRewriter.cpp */

#ifndef BINARY_REWRITER_H
#define BINARY_REWRITER_H

#include "rose.h"

/* Class declaration */
class BinaryRewriter {
    public:
        //Constructor
        //BinaryRewriter();
        //Constructor with file
        BinaryRewriter(char *binaryFile);
        //Function that takes the path to the binary that is to be read.
        //void readBinary(*char);
        //Configure register allocation
        //void selectRegisterAllocation();
        //Configure instruction scheduling
        //void selectInstructionScheduling();

    private:
        /* Pointer to the project AST */
        SgProject *binaryProject;
        // CFG implementation

};


#endif 

