/* Code for handling the symbolic registers. */
#ifndef SYMBOLICREGISTER_H
#define SYMBOLICREGISTER_H

/* header files */
#include "rose.h"


/* function forward declarations */

/* returns a register expression mapped to a symbolic register. */
SgAsmDirectRegisterExpression generateSymbolicRegister();
/* returns the symbolic register name mapped to the register expression */
unsigned findSymbolicRegister(SgAsmDirectRegisterExpression*);
/* Check if a register is symbolic */
bool isSymbolicRegister(SgAsmDirectRegisterExpression*);
/* initialize and clear the symbolic register map and reset the counter */
void clearSymbolicRegister();

#endif
