/* Code for handling the symbolic registers. */


/* header files */
#include "rose.h"


/* function forward declarations */

/* returns a register expression mapped to a symbolic register. */
SgAsmDirectRegisterExpression generateSymbolicRegister();    //!! need to change return type.
/* returns the symbolic register name mapped to the register expression */
std::string findSymbolicRegister(SgAsmDirectRegisterExpression*); //!!need to change argument, need register.
/* Check if a register is symbolic */
bool isSymbolicRegister(SgAsmDirectRegisterExpression*);



