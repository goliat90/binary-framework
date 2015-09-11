#ifndef BINARYCHANGER_H
#define BINARYCHANGER_H

/*  Tracker used to handle the growth of the binary.
    It will rewrite addresses, correct branch addresses.
    Correction of symbol table entries.
    ELF segment size adjustment. */

/*  Includes. */
#include "cfgHandler.hpp"

class binaryChanger {
    public:
    /*  Take CFG handler pointer so the program cfg is available. */
    binaryChanger(CFGhandler*);

    private:
    /*  Hide default constructor. */
    binaryChanger();

    /*  Private variables. */
    CFGhandler* cfgObject;

};

#endif
