/******************************************************************************
* Contains information relevant to instruction which might be needed
* by the framework.
******************************************************************************/

/* header file */
#include "InstructionInformation.hpp"


/* Constructor that initializes the class */
InstructionInfo::InstructionInfo() {
    //Initialize the operand information map 
    //mips_abs_s
    //mips_abs_d
    //mips_abs_ps
    operandMap.insert(std::pair<MipsInstructionKind, std::pair<int, int> >(mips_add, 1, 2);
    //mips_add_s,mips_add_d,mips_add_ps
    operandMap.insert(std::pair<MipsInstructionKind, std::pair<int, int> >(mips_addi, 1, 1);
}


/* Map containing information about the operands for instructions. */
void InstructionInfo::query(MipsInstructionKind inst) {


    
}


