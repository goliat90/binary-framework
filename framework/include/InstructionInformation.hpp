/* Header file for InstructionInformation */

#ifndef INSTRUCTIONINFORMATION_H
#define INSTRUCTIONINFORMATION_H

/* Headers */
#include "rose.h"


//Type of instruction format
enum instructionType {
    typeR,  //one output reg, two input regs.
    typeI,  //one output reg, one input reg, constant...
    typeJ   //jump address.
};


/* Contains information about certain instructions */
class InstructionInfo {
    //constructor
    InstructionInfo();
    
    //query function for function.
    void query(MipsInstructionKind);

    private:
    //container for operands information, the first int is output
    //registers and the second is input registers.
    std::map<MipsInstructionKind, std::pair<int, int> > operandMap;
};



#endif
