/* Naive transformation implementation.  */

#include "naiveTransform.hpp"


/*  Check if instructions have been inserted. Between original instruction
    and the next original instruction. Count the number of symbolic registers
    used. Find the highest amount of symbolic variables used. This tells us
    how much the stack needs to be modified. */

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
    /*  */
    maximumSymbolicsUsed = 0;
    /*  */
    usesAcc = false;
    /*  */
    offset = 0;
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
    //TODO do stack modification here?
}

/* Goes applies the naive transformation in a basic block. */
void naiveHandler::naiveBlockTransform(SgAsmBlock* block) {
    /*  transformed vector for instructions, will be swaped with the old */
    SgAsmStatementPtrList transformedInstructionVector;
    /*  List used to store a region of inserted instructions. */
    std::list<SgAsmStatement*> regionList;
    /*  The blocks statement list is vector based. That is not suitable during
        transforming. Instead move it over to a list. */
    SgAsmStatementPtrList& instructionVector = block->get_statementList();

    /*  Iterate over the list and find regions of inserted instructions.
        Check a region for symbolic registers and exhange them for hard registers. */
    for(SgAsmStatementPtrList::iterator instIter = instructionVector.begin();
        instIter != instructionVector.end(); ++instIter) {
        /*  Go through the instructions until a inserted instruction is found.
            Then transform that region. */
        if ((*instIter)->variantT() == V_SgAsmMipsInstruction) {
            /* Cast to mips instruction pointer */
            SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*instIter);
            instructionStruct decodedMips = decodeInstruction(mips);
            /* When a inserted instruction is found add it to the region list. */
            if (decodedMips.address == 0) {
                regionList.push_back(*instIter);
            } else {
                /*  When a original instruction is found it is saved in the vector. */
                transformedInstructionVector.push_back(*instIter);
                /* if the region list is not empty then we have a region to perform allocation on. */
                if (regionList.empty() == false) {
                    /* There is a region to perform allocation on, since the list is not empty. */ 
                    regionAllocation(&regionList); 
                    /* copy over the regionList to the transformed vector. */
                    for(std::list<SgAsmStatement*>::iterator listIter = regionList.begin();
                        listIter != regionList.end(); ++listIter) {
                        transformedInstructionVector.push_back(*listIter);
                    }
                    /* clear the regionList. */
                    regionList.clear();
                }
            }
        }
    }
    /* Check if the instruction block ended with inserted instructions */
    if (regionList.empty() == false) {
        /* There is a region to perform allocation on, since the list is not empty. */ 
        regionAllocation(&regionList); 
        /* Copy over the regionList */
        for(std::list<SgAsmStatement*>::iterator listIter = regionList.begin();
            listIter != regionList.end(); ++listIter) {
            transformedInstructionVector.push_back(*listIter);
        }
    }
    /* Swap the transformedinstructionvector with the one in the basic block */
    instructionVector.swap(transformedInstructionVector);
}

/*  Transforms a region of inserted instructions so they have real registers */
void naiveHandler::regionAllocation(std::list<SgAsmStatement*>* regionList) {//, SgAsmStatementPtrList* instVector) {
    /*  We know the maximum number of registers that will be used
        by using the maximum symbolics */
    std::map<unsigned, mipsRegisterName> symbolicToHard;
    /*  Initialize the set of registers available for allocation */
    initHardRegisters();
    /* Go through the instructions and exchange the symbolic registers for hard. */
    for(std::list<SgAsmStatement*>::iterator regionIter = regionList->begin();
        regionIter != regionList->end(); ++regionIter) {
        /*  Cast an instruction to mips */
        SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*regionIter);
        /*  Get the operand list and go through the operands looking for register expressions. */
        SgAsmExpressionPtrList& opList = mips->get_operandList()->get_operands();
        /* Then check if an operand is a register, iterater over the vector. */
        for(SgAsmExpressionPtrList::iterator opIter = opList.begin();
            opIter != opList.end(); ++opIter) {
            /* Check if the an operand is a register expression by looking at variantT. */
            if ((*opIter)->variantT() == V_SgAsmDirectRegisterExpression) {
                /* Decode expression and check if it is symbolic */
                registerStruct rStruct = decodeRegister(*opIter);
                /* Check if the register is a symbolic. */
                if (rStruct.regName == symbolic_reg) {
                    /* Symbolic reg. Does it have a hard register or should it get one? */
                    if (symbolicToHard.count(rStruct.symbolicNumber) > 0) {
                        /* It has a hard register. Build a register expression and replace */
                        rStruct.regName = symbolicToHard.find(rStruct.symbolicNumber)->second;
                        SgAsmDirectRegisterExpression* hardReg = buildRegister(rStruct);
                        /* Replace the symbolic register expression */
                        (*opIter) = hardReg;
                    } else {
                        /* The symbolic register has not been given a hard register so do it now. */
                        rStruct.regName = getHardRegister(); 
                        /* Create mapping between symbolic and har register */
                        symbolicToHard.insert(std::pair<unsigned, mipsRegisterName>(rStruct.symbolicNumber, rStruct.regName));
                        /* build the register expression and replace the temporary symbolic(zero). */
                        SgAsmDirectRegisterExpression* hardReg = buildRegister(rStruct);
                        (*opIter) = hardReg;
                    }
                }
            }
        }
    }
    /*  The operands that were symbolic register have now been replaced with
        hard registers. Now load and store instructions are to be insterted. */
    /* reset the offset counter for the naive stack */
    offset = 0;

    if (usesAcc) {
        /* Save the accumulator register */
        mipsRegisterName moveReg = symbolicToHard.begin()->second;
        saveAccumulator(regionList, moveReg);
    }

    //TODO try saving the begining and end iterator before inserting load and stores.
    //this is for when fixing store and save for the accumulator.
//    std::list<SgAsmStatement*>::iterator storeBoundary = regionList->begin();
//    std::list<SgAsmStatement*>::reverse_iterator loadBoundary = regionList->rbegin();
    
    //TODO iterate through the symbolic to hard map and push to stack.
    for(std::map<unsigned, mipsRegisterName>::reverse_iterator symIter = symbolicToHard.rbegin();
        symIter != symbolicToHard.rend(); ++symIter) {
        //TODO for each symbolic and its hard register we create a load instruction track an offset.
        /*  Instruction struct for the store instructions. */
        instructionStruct storeInst;
        storeInst.kind = mips_sw;
        storeInst.mnemonic = "sw";
        storeInst.format = getInstructionFormat(mips_sw);;
        // set sp in the source register
        registerStruct spStruct;
        spStruct.regName = sp;
        storeInst.sourceRegisters.push_back(spStruct);
        // set the source register, which is being saved
        registerStruct source;
        source.regName = symIter->second;
        storeInst.sourceRegisters.push_back(source);
        // set the data size and signbits.
        storeInst.memoryReferenceSize = 32;
        storeInst.significantBits = 32;
        storeInst.isSignedMemory = true;
        // set the constant for the instruction. Then increment it.
        //TODO verify that this offest is correct.
        storeInst.instructionConstant = offset;
        /* Build the instruction */
        SgAsmMipsInstruction* mipsStore = buildInstruction(&storeInst);
        /* Insert it in the beginning of the region */
        regionList->push_front(mipsStore);

        /* Instruction struct for load instructions */
        instructionStruct loadInst;
        loadInst.kind = mips_lw;
        loadInst.mnemonic = "lw";
        loadInst.format = getInstructionFormat(mips_lw);
        // set the destination register that is being restored.
        registerStruct destination;
        destination.regName = symIter->second;
        loadInst.destinationRegisters.push_back(destination);
        // set the sp in the source register
        loadInst.sourceRegisters.push_back(spStruct);
        // set the data size and sign bits.
        loadInst.memoryReferenceSize = 32;
        loadInst.significantBits = 32;
        loadInst.isSignedMemory = true;
        //set the constant of the instruction. then increment it.
        loadInst.instructionConstant = offset;
        offset += 4;
        //build load instruction
        SgAsmMipsInstruction* mipsLoad = buildInstruction(&loadInst);
        //std::cout << "test decode" << std::endl;
        //instructionStruct test = decodeInstruction(mipsLoad);
        // insert it in the end of the region
        regionList->push_back(mipsLoad);
    }


    //Afterward the register can be used to restore the accumulator
    //before that register is restored.

}

/*  This function adds the necessary instructions to save and restore the
    accumulator register */
void naiveHandler::saveAccumulator(std::list<SgAsmStatement*>* regionList, mipsRegisterName tempReg) {
    //Use one of the saved registers to save acc before entering the code region.

    /*  Instruction struct for the store instructions of the low register. */
    instructionStruct storeLo;
    storeLo.kind = mips_sw;
    storeLo.mnemonic = "sw";
    storeLo.format = getInstructionFormat(mips_sw);;
    // set sp in the source register
    //TODO make one that can be reused for loop and here?
    registerStruct spStruct;
    spStruct.regName = sp;
    storeLo.sourceRegisters.push_back(spStruct);
    //set the destination register which is one from symbolicToHard, can be used with both instructions.
    registerStruct moveReg;
    moveReg.regName = tempReg;
    // set the source register, which is being saved
    storeLo.sourceRegisters.push_back(moveReg);
    // set the data size and signbits.
    storeLo.memoryReferenceSize = 32;
    storeLo.significantBits = 32;
    storeLo.isSignedMemory = true;
    // set the constant for the instruction. Then increment it.
    //TODO verify that this offest is correct.
    storeLo.instructionConstant = offset;
    /* Build the instruction */
    SgAsmMipsInstruction* mipsStoreLo = buildInstruction(&storeLo);
    /* Insert it in the beginning of the region */
    regionList->push_front(mipsStoreLo);

    /* move from low instruction */
    instructionStruct mflo;
    mflo.kind = mips_mflo;
    mflo.mnemonic = "mflo";
    mflo.format = getInstructionFormat(mips_mflo);
    //set the register as a destination register.
    mflo.destinationRegisters.push_back(moveReg);
    //build instruction and insert.
    SgAsmMipsInstruction* mipsMflo = buildInstruction(&mflo);
    regionList->push_front(mipsMflo);

    /* load instruction for low register */
    instructionStruct loadLow;
    loadLow.kind = mips_lw;
    loadLow.mnemonic = "lw";
    loadLow.format = getInstructionFormat(mips_lw);
    /* register to put memory value into */
    loadLow.destinationRegisters.push_back(moveReg);
    /* set the sp as the source register */
    loadLow.sourceRegisters.push_back(spStruct);
    loadLow.memoryReferenceSize = 32;
    loadLow.significantBits = 32;
    loadLow.isSignedMemory = true;
    //set the constant of the instruction. then increment it.
    loadLow.instructionConstant = offset;
    offset =+ 4;
    //build and insert the instruction.
    SgAsmMipsInstruction* lwLow = buildInstruction(&loadLow);
    regionList->push_back(lwLow);


    /*  Instruction struct for the store instructions of the low register. */
    instructionStruct storeHi;
    storeHi.kind = mips_sw;
    storeHi.mnemonic = "sw";
    storeHi.format = getInstructionFormat(mips_sw);;
    // set sp in the source register
    //TODO make one that can be reused for loop and here?
    storeHi.sourceRegisters.push_back(spStruct);
    // set the source register, which is being saved
    storeHi.sourceRegisters.push_back(moveReg);
    // set the data size and signbits.
    storeHi.memoryReferenceSize = 32;
    storeHi.significantBits = 32;
    storeHi.isSignedMemory = true;
    // set the constant for the instruction. Then increment it.
    //TODO verify that this offest is correct.
    storeHi.instructionConstant = offset;
    offset =+ 4;
    /* Build the instruction */
    SgAsmMipsInstruction* mipsStoreHi = buildInstruction(&storeHi);
    /* Insert it in the beginning of the region */
    regionList->push_front(mipsStoreHi);

    //mfhi instruction struct.
    instructionStruct mfhi;
    mfhi.kind = mips_mfhi;
    mfhi.mnemonic = "mfhi";
    mfhi.format = getInstructionFormat(mips_mfhi);
    //reuse the register from the mflo instruction.
    mfhi.destinationRegisters.push_back(moveReg);
    //build instruction and insert
    SgAsmMipsInstruction* mipsMfhi = buildInstruction(&mfhi);
    regionList->push_front(mipsMfhi);

    /* saving accumulator instructions added. Now for restoring it. */
}


/*  Initalize the register set used during a region allocation */
void naiveHandler::initHardRegisters() {
    /*  Set stack offset count to zero */
    usedHardRegs = 0;
    /*  Clear the set */
    hardRegisters.clear();
    /*  Add registers to the set */
    hardRegisters.insert(t0);
    hardRegisters.insert(t1);
    hardRegisters.insert(t2);
    hardRegisters.insert(t3);
    hardRegisters.insert(t4);
    hardRegisters.insert(t5);
    hardRegisters.insert(t6);
    hardRegisters.insert(t7);
}

/*  Help function that will return hard registers for exchange.
    It will also create a offset for the load and store instructions. */
mipsRegisterName naiveHandler::getHardRegister() {
    /* Variable */
    mipsRegisterName realReg;
    /*  Check if the set contains registers. */
    if (!hardRegisters.empty()) {
        /*  Get a register from the set */
        realReg = *hardRegisters.begin();
        /*  Remove the register since it is now used */
        hardRegisters.erase(hardRegisters.begin());
        /* increment the counter */
        usedHardRegs++;
    } else {
        /*  No more registers available. Failure */
        abort();
    }
    return realReg;
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
                    for(std::vector<registerStruct>::iterator regIter = decodedInst.destinationRegisters.begin();
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
                    for(std::vector<registerStruct>::iterator regIter = decodedInst.sourceRegisters.begin();
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
            /*  Check if the counter is zero, then we need to increment it once extra.
                Special case since we need to an extra register to be used when saving ACC. */
            if (*currentModification == 0) {
                *currentModification++;
            }
            *currentModification += 2;
            /*  Set that the acc register needs to be saved */
            usesAcc = true;
            break;
        }
        default: {
            /* Default instructions does not need any consideration */
        }
    }
}


