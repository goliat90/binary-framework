/* Debug functions for the framework such as print functions */

#ifndef BINARYDEBUG_H
#define BINARYDEBUG_H 

/* Headers. */
#include "rose.h"
/* get instruction decoding functions.  */
#include "mipsISA.hpp"
/* string stream */
#include <sstream>
#include <string>


/* function declarations  */
/*  Prints the content i.e. the instructions of a basic block. */
void printBasicBlockInstructions(SgAsmBlock*);
/*  Wrapper call for the printInstruction but you pass SgAsmMipsInstruction pointers. */
void printInstruction(SgAsmMipsInstruction*);
/*  Prints a specific instruction */
void printInstruction(instructionStruct*);
/*  Get the string for a register. */
std::string getRegisterString(mipsRegisterName);

/*  Print the content of a block in assembly format. */
void printBasicBlockAsAssembly(SgAsmBlock*);
/*  Print instruction in functional assembly format. */
void printAssemblyInstruction(SgAsmMipsInstruction*);

#endif

