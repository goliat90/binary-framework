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
void binaryChanger::preTransformAnalysis() {
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

        /*  Get the address of the block and store it. */

        /*  Get the address of the last instruction in the block and store it. */

    }

    /*  All the basic blocks have been inserted. Sort the 
        vector according to blocks address size. */
    std::sort(basicBlockVector.begin(), basicBlockVector.end(), blockSortStruct());
}


/*  Post tranformation function. Fixes the binary so it
    is fixed. */
void binaryChanger::postTransformationWork() {

}



