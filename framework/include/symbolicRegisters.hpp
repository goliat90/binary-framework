/* Code for handling the symbolic registers. */
#ifndef SYMBOLICREGISTER_H
#define SYMBOLICREGISTER_H

/* header files */
#include "rose.h"
#include "boost/bimap.hpp"

/* forward declarations */
struct registerStruct;

/* returns a register expression mapped to a symbolic register. */
registerStruct generateSymbolicRegister();
/* Returns the directregisterexpression connected to a specific symbolic register */
SgAsmDirectRegisterExpression* getDirectRegisterExpression(unsigned); 
/* returns the symbolic register name mapped to the register expression */
unsigned findSymbolicRegister(SgAsmDirectRegisterExpression*);
/* Check if a register is symbolic */
bool isSymbolicRegister(SgAsmDirectRegisterExpression*);
/* initialize and clear the symbolic register map and reset the counter */
void clearSymbolicRegister();

//name of the registers.
enum mipsRegisterName {
    zero,           //always zero, if zero is encountered then check if it is symbolic.
    at,             //assembly temporary
    v0,v1,          //return value from function call.

    a0,a1,a2,a3,    //first four parameters of function call

    t0,t1,t2,t3,    //temporary variables, no need to preserve
    t4,t5,t6,t7,

    s0,s1,s2,s3,    //function variables, must be preserved.
    s4,s5,s6,s7,

    t8,t9,          //more temporary variables
    k0,k1,          //kernel use register, might change unexpectedly.
    gp,             //global pointer
    sp,             //stack pointer
    fp,             //stack frame pointer or subroutine variable(s8)
    ra,             //return address of the last subroutine call.

    //hi,lo,          //special register for multiplication and division.
                    //not included in the lookups.
    symbolic_reg    //sumbolic registers.
};


/* struct for register information. Contains enum and symbolic number */
struct registerStruct {
    //constructor
    registerStruct():regName(symbolic_reg), symbolicNumber(0){};
    //members
    mipsRegisterName regName;
    unsigned symbolicNumber;
};


#endif

