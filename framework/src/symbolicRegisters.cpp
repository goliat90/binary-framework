/* Handles symbolic registers in the framework */

/* Header file */
#include "symbolicRegisters.hpp"
#include <sstream>

/* Forward declaration */
unsigned generateRegName();

/* map to connect the register expression object to the symbolic register. */
static std::map<SgAsmDirectRegisterExpression*, unsigned> symbolicRegisterMap;
typedef std::pair<SgAsmDirectRegisterExpression*, unsigned> mapPair;

/* Create a symbolic register that can be used */
SgAsmDirectRegisterExpression generateSymbolicRegister() {
    /* Create the register descriptor, it references zero. */
    RegisterDescriptor rd = RegisterDescriptor(mips_regclass_gpr, 0, 0, 0); 
    /* Create the register expression. */
    SgAsmDirectRegisterExpression regExp = SgAsmDirectRegisterExpression(rd);
    /* Get a sym register name (number) */
    unsigned regName = generateRegName();
    /* Insert the register to the map with associated register */
    symbolicRegisterMap.insert(mapPair(&regExp, regName));
    /* Return the register expression */ 
    return regExp;
}

/* Generates the symbolic register name, which is just a number. */
unsigned generateRegName() {
    /* Number counter for the symbolic regs */
    static unsigned symbolicNumber = 0;
    /* Increment the number */
    symbolicNumber++;
    /* return the number*/
    return symbolicNumber;
}

/* Checks if a registerexpression is symbolic or not. */
bool isSymbolicRegister(SgAsmDirectRegisterExpression* reg) {
    /* Check if the register expression is present in the map. */
    if (symbolicRegisterMap.find(reg) != symbolicRegisterMap.end()) {
        //the register is present in the map so it is symbolic.
        return true;
    } else {
        //the register was not found, so it is a real register that is used.
        return false;
    }
}

/* search for a symbolic register. */
unsigned findSymbolicRegister(SgAsmDirectRegisterExpression* reg) {
    //get the symbolic name of the register.
    unsigned symRegName = symbolicRegisterMap.find(reg)->second;
    //return the name.
    return symRegName;
}


