#ifndef BINARYCHANGER_H
#define BINARYCHANGER_H

/*  Tracker used to handle the growth of the binary.
    It will rewrite addresses, correct branch addresses.
    Correction of symbol table entries.
    ELF segment size adjustment. */

/*  Includes. */
#include "cfgHandler.hpp"
#include "binaryDebug.hpp"
#include "boost/bimap.hpp"
#include "boost/bimap/set_of.hpp"
#include "boost/bimap/multiset_of.hpp"
#include "rose.h"


/*  Struct with overloaded () operator used for sorting the basic blocks. */
struct blockSortStruct {
    bool operator()(SgAsmBlock* i, SgAsmBlock* j) const {
        return (i->get_address() < j->get_address());
    }
};

/*  Sort segments or sections according to their virtual address. */
struct elfSectionSortStruct {
    bool operator()(SgAsmElfSection* i, SgAsmElfSection* j) const {
        return (i->get_mapped_preferred_va() < j->get_mapped_preferred_va());
    }
};

/*  Sort segments according to the file offset. */
struct elfSectionFileSortStruct {
    bool operator()(SgAsmElfSection* i, SgAsmElfSection* j) const {
        return (i->get_offset() < j->get_offset());
    }
};

/* typedefs. */
typedef std::vector<SgAsmElfSection*> asmElfVector; 
typedef boost::bimap<boost::bimaps::set_of<SgAsmElfSection*>, 
                    boost::bimaps::multiset_of<rose_addr_t, std::greater<rose_addr_t> > > segDiffType;
typedef boost::bimap<rose_addr_t, SgAsmElfSection*> addressVoidType; 


/*  Class declaration. */
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

    /*  Analysis functions used before transformations. */
    /*  Collect information about basic blocks. */
    void preBlockInformationCollection();
    /*  Collect information about sections and segments. */
    void preSegmentSectionCollection();
    /*  Map branches targets. */
    void preBranchAnalysis();


    /*  Functions used after transformations applied. */
    /*  Check how blocks have changed due to transformations and
        the segments they belong to. Not the sizes. */
    void postChanges();
    /*  Function to reallocate segment and start adjusting their size. */
    void reallocateSegments();
    /*  Find open spaces within the virtual address space.
        This is to find suitable open spaces to move segments. */
    void findFreeVirtualSpace();
    /*  For each moved segment find the basic blocks that belong to it
        and assign them new addresses. */
    void moveSegmentBasicBlocks();
    /*  Correct the branch instructions in the basic blocks. */
    void redirectBranchInstructions();


    /*  Private variables. */
    /*  Debugging variable. */
    bool debugging;
    /*  Address bound addresses. */
    rose_addr_t lowerVirtualAddressLimit;
    rose_addr_t upperVirtualAddressLimit;
    /*  Lowest allowed segment address. It will be the address
        of the first segment. */
    rose_addr_t firstSegmentAddress;

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
    /*  Map containing an segments previous address. Need it to keep track of
        which basic blocks belong to the segment and should have a new address.
        Also to ensure that i keep any gap between the start address of the segment
        and the first instruction. */
    std::map<SgAsmElfSection*, rose_addr_t> segmentOldAddr;
    /*  For for the old size. */
    std::map<SgAsmElfSection*, rose_addr_t> segmentOldSize;
    /*  Hashmap containing information regarding if there
        is space between a two segments. If a segment is present
        then there is some address space available between the
        segment and the next one. */
    boost::bimap<rose_addr_t, SgAsmElfSection*> addressVoids;

    /*  Bimap for storing branches and their initial target. */
    std::map<SgAsmInstruction*, rose_addr_t> branchTargetMap;

    /*  Container for the block pointers.
        It will be sorted according to address. */
    std::vector<SgAsmBlock*> basicBlockVector;
    /*  Map for the blocks original starting address. I assume it is
        the blocks address or the first instructions. */
//    std::map<SgAsmBlock*, rose_addr_t> blockStartAddrMap;
    boost::bimap<SgAsmBlock*, rose_addr_t> blockStartAddrMap;
    /*  Map for the blocks original end address. */
    std::map<SgAsmBlock*, rose_addr_t> blockEndAddrMap;
    /*  Map between old addresses and new. The only addresses
        that are saved are for the basic blocks there is no
        reason to save an address for an instruction in the
        middle of the block. */
    std::map<rose_addr_t, rose_addr_t> oldToNewAddrMap;

    /*  Map storing the original block size of blocks.
        Retrieved before transforming. */
    std::map<SgAsmBlock*, int> blockOriginalSize;
    /*  The difference in size of basic blocks, if their size has changed
        then they have an entry here. The numbers here is not bytes
        but the number of instructions added or removed. */
    std::map<SgAsmBlock*, int> blockSizeDifference;
    /*  Segment difference for section. The diff value is the number of bytes
        that the inserted instructions requires. */
    //TODO need to consider changing rose_addr_t to a signed type. Otherwise how do i handle shrinkage?
    boost::bimap<boost::bimaps::set_of<SgAsmElfSection*>, 
                    boost::bimaps::multiset_of<rose_addr_t, std::greater<rose_addr_t> > > segmentSizeDifference;

};

#endif
