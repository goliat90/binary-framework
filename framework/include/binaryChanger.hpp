#ifndef BINARYCHANGER_H
#define BINARYCHANGER_H

/*  Tracker used to handle the growth of the binary.
    It will rewrite addresses, correct branch addresses.
    Correction of symbol table entries.
    ELF segment size adjustment. */

/*  Includes. */
#include "cfgHandler.hpp"
#include "rose.h"

class binaryChanger {
    public:
    /*  Take CFG handler pointer so the program cfg is available. */
    binaryChanger(CFGhandler*);

    private:
    /*  Hide default constructor. */
    binaryChanger();

    /*  Private variables. */
    /*  Cfg handler pointer. */
    CFGhandler* cfgObject;
    /*  Container for the block pointers. */

    /*  Container map old addresses from new. */
    //TODO Consider hving this as a boost bimap. could i need mapping between new and old?
    std::map<rose_addr_t, rose_addr_t> oldToNewAddrMap;

    /*  Container to track a basic blocks end address. */
    //TODO knowing the end address might be essential to be able to handle address gaps in the block.
    std::map<rose_addr_t, rose_addr_t> blockGrowth;

    /*  Track the size or end address of basic blocks... */
    //TODO knowing how much an block has grown will be useful.
};

#endif
