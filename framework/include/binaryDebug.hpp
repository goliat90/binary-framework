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
/*  Prints a specific instruction */
void printInstruction(instructionStruct*);

#endif

