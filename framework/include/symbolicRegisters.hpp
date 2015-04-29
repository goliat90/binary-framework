/* Code for handling the symbolic registers. */
#ifndef SYMBOLICREGISTER_H
#define SYMBOLICREGISTER_H

/* header files */
#include "rose.h"
#include "boost/bimap.hpp"

/* function forward declarations */

/* returns a register expression mapped to a symbolic register. */
unsigned generateSymbolicRegister();
/* Returns the directregisterexpression connected to a specific symbolic register */
SgAsmDirectRegisterExpression* getDirectRegisterExpression(unsigned); 
/* returns the symbolic register name mapped to the register expression */
unsigned findSymbolicRegister(SgAsmDirectRegisterExpression*);
/* Check if a register is symbolic */
bool isSymbolicRegister(SgAsmDirectRegisterExpression*);
/* initialize and clear the symbolic register map and reset the counter */
void clearSymbolicRegister();

#endif
