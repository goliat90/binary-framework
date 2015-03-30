/* Handles symbolic registers in the framework */

/* Header file */
#include "symbolicRegisters.hpp"
#include <string>

/* Number counter for the symbolic regs */
static int symbolicNumber = 0;
/* map to connect the register expression object to the symbolic register. */
static std::map<SgAsmDirectRegisterExpression*, std::string> symbolicRegisterMap;

/* Create a symbolic register that can be used */
SgAsmDirectRegisterExpression generateSymbolicRegister() {
    /* Create the register descriptor, it references zero. */
    RegisterDescriptor rd = RegisterDescriptor(mips_regclass_gpr, 0, 0, 0); 
    /* Create the register expression. */
    SgAsmDirectRegisterExpression regExp = SgAsmDirectRegisterExpression(rd);
    /* Create the symbolic register name */
    //to_string will probably not work. need to add files to make.
    //std::string symReg = "symReg_" + std::to_string(symbolicNumber);
    /* Insert the register to the map with associated register */
    
    /* Return the register expression */ 
    
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
std::string findSymbolicRegister(SgAsmDirectRegisterExpression* reg) {
    //get the symbolic name of the register.
    std::string symRegName = symbolicRegisterMap.find(reg)->second;
    //return the name.
    return symRegName;
}


