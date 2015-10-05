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
    binaryChanger(CFGhandler*, SgProject*);
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
    /*  Pointer for the SgProject. It is available here for
        to be able to extract the segments and other information. */
    SgProject* changerProjectPtr;
    /*  Cfg handler pointer. */
    CFGhandler* cfgObject;
    /*  A vector containing all elf sections present. */
    std::vector<SgAsmElfSection*> elfSections;
    /*  Vector of pointers to segments that are sections. */
    std::vector<SgAsmElfSection*> sectionVector;
    /*  Vector of pointers to relevant segments.
        Will contain all segments, including data.
        This is to have a entire picture. */
    std::vector<SgAsmElfSection*> segmentVector;
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
