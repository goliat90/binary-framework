#ifndef BINARYCHANGER_H
#define BINARYCHANGER_H

/*  Tracker used to handle the growth of the binary.
    It will rewrite addresses, correct branch addresses.
    Correction of symbol table entries.
    ELF segment size adjustment. */

/*  Includes. */
#include "cfgHandler.hpp"
#include "rose.h"


/*  Struct with overloaded () operator used for sorting the basic blocks. */
struct blockSortStruct {
    bool operator()(SgAsmBlock* i, SgAsmBlock* j) const {
        return (i->get_address() < j->get_address());
    }
};

class binaryChanger {
    public:
    /*  Take CFG handler pointer so the program cfg is available. */
    binaryChanger(CFGhandler*);
    /*  Initial analysis of the of the binary before the transformation. */
    void preTransformationAnalysis();
    /*  Post transformation work. */
    void postTransformationWork();
    /*  Sets debug mode. */
    void setDebugging(bool mode) {debugging = mode;};

    private:
    /*  Hide default constructor. */
    binaryChanger();

    /*  Private variables. */
    /*  Debugging variable. */
    bool debugging;
    /*  Cfg handler pointer. */
    CFGhandler* cfgObject;
    /*  Container for the block pointers.
        It will be sorted according to address. */
    std::vector<SgAsmBlock*> basicBlockVector;

    /*  Map for the blocks starting address. I assume it is
        the blocks address or the first instructions. */
    std::map<SgAsmBlock*, rose_addr_t> blockStartAddrMap;
    /*  Map for the blocks end address. */
    std::map<SgAsmBlock*, rose_addr_t> blockEndAddrMap;

    /*  Map between old addresses and new. */
    std::map<rose_addr_t, rose_addr_t> oldToNewAddrMap;

    /*  Map storing the original block size of blocks.
        Retrieved before transforming. */
    std::map<SgAsmBlock*, int> blockOriginalSize;
    /*  The new size of basic blocks, if their size has changed
        then they have an entry here. */
    std::map<SgAsmBlock*, int> blockNewSize;

    //TODO some kind of structure to store the segments, possibly ordered.
    //TODO If it is ordered i could iterate it and change accordingly. 
    //TODO perhaps have structure for after changes have been done.
};

#endif
