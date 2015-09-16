/*  Handles the growth or shrinkage of a binary.
    Rewrites addresses.
    Corrects branch addresses.
    Symbol table entries fixed.
    Segment sizes changed.
*/

/*  Steps that need to be done.
    1. change the addresses for the subsequent instructions.
    2. fix up target addresses for branch instructions
    3. fix symbol tables to point to new addresses
    4. extend an existing ELF segment or add a new one
    5. move later segments to higher addresses
    6. fix the ELF segment table
*/

/*  Achieve the steps above i need to analyze the binary
    before applying transform. Basically get a state
    of the original binary. After transforms has been applied
    the new segment sizes are calculated. Last is to
    apply the address rewriting steps. In summary.
    
    1. Analyze the binary before transformation to get
    all needed information to perform changes correctly.

    2. After transforms calculate the new segment sizes.

    3. Rewrite the addresses of instructions, branches,
    fix symbol tables.
*/

/*  Include. */
#include "binaryChanger.hpp"


/*  Constructor. */
binaryChanger::binaryChanger(CFGhandler* cfgH) {
    /*  Save the cfg handler pointer. */
    cfgObject = cfgH;
    /*  set debugging to false as default. */
    debugging = false;

}


/*  This function performs analysis on the binary before
    the transformations are applied. */
void binaryChanger::preTransformationAnalysis() {
    /*  Get the program cfg pointer. */
    CFG* entireCFG = cfgObject->getProgramCFG();
    /*  Go through the program CFG and extract all the basic blocks. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*entireCFG);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Extract the basic block. */
        SgAsmBlock* nodeBlock = get(boost::vertex_name, *entireCFG, *iterPair.first);
        /*  Store the block in the vector. */
        basicBlockVector.push_back(nodeBlock);
        /*  Retrieve the size of the basic block, that is the
            number of instructions in the block. */
        SgAsmStatementPtrList& stmtList = nodeBlock->get_statementList();
        int blockSize = stmtList.size();
        blockOriginalSize.insert(std::pair<SgAsmBlock*, int>(nodeBlock, blockSize));

        /*  Get the address of the block and store it. */
        blockStartAddrMap.insert(std::pair<SgAsmBlock*, rose_addr_t>(nodeBlock, nodeBlock->get_id()));
        /*  Get the address of the last instruction in the block and store it. */
        rose_addr_t lastAddress;
        if (!stmtList.empty()) {
            /*  There are statements so get the last one. */
            SgAsmStatement* lastStatement = stmtList.back();
            lastAddress = lastStatement->get_address();
        } else {
            /*  The block is empty. The last address is then the first address. */
            if (debugging) {
                std::cout << "Found a block with no instructions." << std::endl;
                lastAddress = nodeBlock->get_id();
            }
        }
        blockEndAddrMap.insert(std::pair<SgAsmBlock*, rose_addr_t>(nodeBlock, lastAddress));

        /*  Debug print. */
//        if (debugging) {
//            std::cout << "Block with start address: "
//                << std::showbase << std::hex << nodeBlock->get_id() << std::endl
//                << "last address: " << std::hex << lastAddress << std::endl
//                << "size: " << std::dec << blockSize << std::endl
//                << "stored." << std::endl;
//        }
    }
    if (debugging) {
        std::cout << "Information about " << basicBlockVector.size() << " blocks collected." << std::endl;
    }

    /*  All the basic blocks have been inserted. Sort the 
        vector according to blocks address size. */
    std::sort(basicBlockVector.begin(), basicBlockVector.end(), blockSortStruct());

    /*  At this point i have a orderded vector of basic blocks, from low to high.
        For each block i know the starting address and ending address.
        The original size of each block is known. */
    //TODO i should get access to the segments here, or even earlier.
}


/*  Post tranformation function. Fixes the binary so it
    is fixed. */
void binaryChanger::postTransformationWork() {

}



