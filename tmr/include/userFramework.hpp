#ifndef USERFRAMEWORK_H
#define USERFRAMEWORK_H


//Include framework header.
#include "binaryRewriter.hpp"

/*
This class is used by the user to define the behavior of transformations.
It is passed to the framework which calls it during traversal of the binary.
*/
class userFramework : public BinaryRewriter {
    public:
        //constructor
        userFramework(int, char**);
        //overriding the decision function in the framework.
        void transformDecision(SgAsmMipsInstruction*);

    private:
        //Hide default constructor again.
        userFramework();
};

#endif
