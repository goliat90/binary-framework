/* Handles symbolic registers in the framework */

/* Header file */
#include "symbolicRegisters.hpp"

/* Forward declaration */
unsigned generateRegName();

/* map to connect the register expression object to the symbolic register. */
typedef boost::bimap<unsigned, SgAsmDirectRegisterExpression*> regBiMap;
static regBiMap biRegisterMap;

/* Number counter for the symbolic regs, starting on 1 since
   0 is the default value for registerstructs. */
static unsigned symbolicNumber = 1;

/* Create a symbolic register that can be used */
registerStruct generateSymbolicRegister() {
    /* Create register struct to return  */
    registerStruct regStruct;
    regStruct.regName = symbolic_reg;
    /* Create the register descriptor, it references zero. */
    RegisterDescriptor rd = RegisterDescriptor(mips_regclass_gpr, 0, 0, 0); 
    /* Create the register expression. */
    SgAsmDirectRegisterExpression* regExp = new SgAsmDirectRegisterExpression(rd);
    /* Get a sym register name (number) */
    regStruct.symbolicNumber = generateRegName();
    /* Insert the register to the map with associated register */
    biRegisterMap.insert(regBiMap::value_type(regStruct.symbolicNumber, regExp));
    /* Return the register number*/ 
    return regStruct;
}

/* Retrieve the DirectRegisterExpression */
SgAsmDirectRegisterExpression* getDirectRegisterExpression(unsigned number) {
    /* search the map for the correct register expression */
    SgAsmDirectRegisterExpression* reg = biRegisterMap.left.find(number)->second;
    /* return the expression */
    return reg;
}


/* Clear the map of registers and set symbolic number value to 0  */
void clearSymbolicRegister() {
    /* set symbolic number to 1 again */
    symbolicNumber = 1;
    /* clear the map */
    biRegisterMap.clear();
}

/* Generates the symbolic register name, which is just a number. */
unsigned generateRegName() {
    /* Increment the number */
    int number = symbolicNumber++;
    /* return the number*/
    return number;
}

/* Checks if a registerexpression is symbolic or not. */
bool isSymbolicRegister(SgAsmDirectRegisterExpression* reg) {
    /* Check if the register expression is present in the map. */
    if (biRegisterMap.right.find(reg) != biRegisterMap.right.end()) {
        return true;
    } else {
        return false;
    }
}

/* search for a symbolic register. */
unsigned findSymbolicRegister(SgAsmDirectRegisterExpression* reg) {
    //get the symbolic name of the register.
    unsigned symRegName = biRegisterMap.right.find(reg)->second;
    //return the name.
    return symRegName;
}


