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
    determineStackModification();
}

/* Function that applies the naive transformation to the binary */
void naiveHandler::applyTransformation() {
    /* Variables */
    CFG* function = cfgContainer->getFunctionCFG();
    /* Find the maximum use of symbolic registers. */
    determineStackModification();
    /* Go through the instructions in a basic block and  */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*function);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /* Get the basicblock from the vertex.  */
        SgAsmBlock* bb = get(boost::vertex_name, *function, *iterPair.first);
        /* Transform the block. */
        naiveBlockTransform(bb);
    }
}

/* Goes applies the naive transformation in a basic block. */
void naiveHandler::naiveBlockTransform(SgAsmBlock* block) {
    /*  transformed vector for instructions, will be swaped with the old */
    SgAsmStatementPtrList transformedInstructionVector;
    /*  The blocks statement list is vector based. That is not suitable during
        transforming. Instead move it over to a list. */
    SgAsmStatementPtrList& instructionVector = block->get_statementList();
    std::list<SgAsmStatement*> instructionList (instructionVector.begin(), instructionVector.end());

    /*  Iterate over the list and find regions of inserted instructions.
        Check a region for symbolic registers and exhange them for hard registers. */
    for(std::list<SgAsmStatement*>::iterator instIter = instructionList.begin();
        instIter != instructionList.end(); ++instIter) {
        /*  Go through the instructions until a inserted instruction is found.
            Then transform that region. */
        
    }
    
}

/* Find the maximum amount of used symbolic registers used at the same time */
void naiveHandler::determineStackModification() {
    /* counter for symbolic registers */
    int maxSymbolics = 0;
    /* Keep track of registers to avoid counted registers. */
    std::set<unsigned> symregsCounted;
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
            /* Check that is is a mips instruction */
            if ((*iter)->variantT() == V_SgAsmMipsInstruction) {
                /* Cast it to mips instruction and decode it. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*iter);
                instructionStruct decodedInst = decodeInstruction(mips);
                /* Check if the instruction is original or inserted.
                    Is done by checking the address. */
                if (decodedInst.address == 0) {
                    /* The instruction is an inserted one, check use of symbolic
                        registers. */
                    for(std::vector<registerStruct>::iterator regIter = 
                        decodedInst.destinationRegisters.begin();
                        regIter != decodedInst.destinationRegisters.end(); ++regIter) {
                        /* reg struct variable */
                        registerStruct reg = (*regIter);
                        /* Check each register struct if it is symbolic */ 
                        if (reg.regName == symbolic_reg &&
                            symregsCounted.count(reg.symbolicNumber) == 0) {
                            /* increment the count */
                            maxSymbolics++;
                            /* Add it to the counted symbolic registers */
                            symregsCounted.insert(reg.symbolicNumber);
                        }
                    }
                    for(std::vector<registerStruct>::iterator regIter = 
                        decodedInst.sourceRegisters.begin();
                        regIter != decodedInst.sourceRegisters.end(); ++regIter) {
                        /* reg struct variable */
                        registerStruct reg = (*regIter); 
                        /* Check each register struct if it is symbolic */ 
                        if (reg.regName == symbolic_reg &&
                            symregsCounted.count(reg.symbolicNumber) == 0) {
                            /* increment the count */
                            maxSymbolics++;
                            /* Add it to the counted symbolic registers */
                            symregsCounted.insert(reg.symbolicNumber);
                        }
                    }
                    /*  Check if the instruction uses special registers,
                        increment according to that case. */
                    specialInstructionUse(decodedInst.kind, &maxSymbolics);
                } else {
                    /* Clear the set and reset the counter whenever a original
                        instruction is encountered again. */
                    symregsCounted.clear();
                    maxSymbolics = 0;
                }
                /* save symbolic count if it is higher than the previously. */
                if (maximumSymbolicsUsed < maxSymbolics) {
                    /* save new maximum symbolics used. */
                    maximumSymbolicsUsed = maxSymbolics;
                }
            }
        }
    }
    std::cout << "maximum symbolics:" << maximumSymbolicsUsed << std::endl;
}

/*  Help function for determining how much the stack needs to be modified,
    The instruction kind will be used to determine how much stack needs to
    modified. */
void naiveHandler::specialInstructionUse(MipsInstructionKind kind, int* currentModification) {
    /* Switch case for instructions */
    switch(kind) {
        /* These instructions uses the accumulator register, Hi and low */
        case mips_madd:
        case mips_maddu:
        case mips_msub:
        case mips_msubu:
        case mips_mult:
        case mips_multu:
        case mips_div:
        case mips_divu: {
            *currentModification += 2;
            break;
        }
        default: {
            /* Default instructions do not need any consideration */
        }
    }
}


