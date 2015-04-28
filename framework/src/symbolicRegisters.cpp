/* Handles symbolic registers in the framework */

/* Header file */
#include "symbolicRegisters.hpp"

/* Forward declaration */
unsigned generateRegName();

/* map to connect the register expression object to the symbolic register. */
static std::map<SgAsmDirectRegisterExpression*, unsigned> symbolicRegisterMap;
typedef std::pair<SgAsmDirectRegisterExpression*, unsigned> mapPair;

/* Number counter for the symbolic regs, starting on 1 since
   0 is the default value for registerstructs. */
static unsigned symbolicNumber = 1;

/* Create a symbolic register that can be used */
unsigned generateSymbolicRegister() {
    /* Create the register descriptor, it references zero. */
    RegisterDescriptor rd = RegisterDescriptor(mips_regclass_gpr, 0, 0, 0); 
    /* Create the register expression. */
    SgAsmDirectRegisterExpression regExp = SgAsmDirectRegisterExpression(rd);
    /* Get a sym register name (number) */
    unsigned regNumber = generateRegName();
    /* Insert the register to the map with associated register */
    symbolicRegisterMap.insert(mapPair(&regExp, regNumber));
    /* Create a symbolic registerStruct with the generated number */
    registerStruct rs;
    rs.symbolicNumber = regNumber;
    /* Return the register expression */ 
    return 0;
}


/* Clear the map of registers and set symbolic number value to 0  */
void clearSymbolicRegister() {
    /* set symbolic number to 1 again */
    symbolicNumber = 1;
    /* clear the map */
    symbolicRegisterMap.clear();
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


