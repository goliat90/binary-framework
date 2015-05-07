/* Naive transformation implementation.  */

#include "naiveTransform.hpp"


/*  Check if instructions have been inserted. Between original instruction
    and the next original instruction. Count the number of symbolic registers
    used. Find the highest amount of symbolic variables used. This tells us
    how much the stack needs to be modified. */


/*  Need a container for the instructions that preserves iterators better
    than std::vector. Looks like std::list might be the best option, it is
    implemented as a double linked list.  */
/*  I need to make a conversion function that translates the vector to list.
    Seem like there is a std::copy i can use OR just the list constructor.  */

/*  For each region of inserted instructions determine how many registers
    are needed for the region. Then insert appropriate load and store
    instructions to free registers. !!! Need to check in some way that special
    registers are used. */

/*  STEPS
        1. Determine how much the stack pointer needs to be changed.
        So count symbolic registers used between original instructions.
        Determine if i can do this in the vectors or if i need a list.
        Remember to consider special registers. HI and LO registers, ACC
        as well.

        1.5 Check each instruction block for inserted instructions.
        If none are found then skip the block. Think it might be a bad
        assumption that some basic blocks are not transformed.

        2. Insert load and stores in regions of inserted instructions.
        Here i could do some kind of mapping between a symbolic register
        and a real register. The instructions need to have correct offests
        as well in load and store. 

        3. Replace symbolic registers with real registers in the current
        region of instructions.

        4. Move on to the next region of inserted instructions and repeat 2-4.

        5. All instructions have been allocated real registers. Transform complete.
*/

/* Constructor available */
naiveHandler::naiveHandler(CFGhandler* handler) {
    /* save the cfg handler pointer */
    cfgContainer = handler;
    /* */
    maximumSymbolicsUsed = 0;
    /* check the use of symbolic registers here maybe? */
}

/* Function that applies the naive transformation to the binary */
void naiveHandler::applyTransformation() {
    /* Find the maximum use of symbolic registers. */

    /* Go through each block and transform them */
}


/* Find the maximum amount of used symbolic registers used at the same time */
void naiveHandler::findSymbolicRegisterUse() {
    /* Go through the cfg and look for the maximum number of used instructions */
    CFG* function = cfgContainer->getFunctionCFG(); 
    /* Get all vertices and their block. */
    for(std::pair<CFGVIter, CFGVIter> vpIter = vertices(*function);
        vpIter.first != vpIter.second; ++vpIter.first) {
        /* Get the basicblock */
        SgAsmBlock* block = get(boost::vertex_name, *function, *vpIter.first);
        /* Get the statement list from the block, get it as a reference. */
        SgAsmStatementPtrList& instList = block->get_statementList();
        /* Go through the instructions and count symbolic registers */
        for(SgAsmStatementPtrList::iterator iter = instList.begin();
            iter != instList.end(); ++iter) {
            /* Check if the instruction is original or inserted. */
        }

    }


}

/* Goes through a basic block and transforms it */
void naiveHandler::naiveBlockTransform(SgAsmBlock* basic) {

}

