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
    /*  Modify the stack. */
    modifyStack();
}

/* Goes applies the naive transformation in a basic block. */
void naiveHandler::naiveBlockTransform(SgAsmBlock* block) {
    /*  transformed vector for instructions, will be swaped with the old */
    SgAsmStatementPtrList transformedInstructionVector;
    /*  List used to store a region of inserted instructions. */
    std::list<SgAsmStatement*> regionList;
    /*  Boolean passed to regionallocation notifying if the
        instruction just before the region writes to acc. In that
        case the acc registers will not be saved. */
    bool preserveAcc = true;
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
                /*  An original instruction was encountered. If there is no region of code
                    the the instructions is added to the regionList. If there is some
                    instructions in the region list then do region allocation. In the
                    worst case it will be region allocation on an alone original instruction.
                    Other cases will be region transform on an original instruction and
                    inserted instructions after it. */
                if (false == regionList.empty()) {
                    /* There is a region to perform allocation on, since the list is not empty. */ 
                    regionAllocation(&regionList, preserveAcc); 
                    /* copy over the regionList to the transformed vector. */
                    transformedInstructionVector.insert(transformedInstructionVector.end(),
                        regionList.begin(), regionList.end());
                    /* clear the regionList. */
                    regionList.clear();
                    /*  Add the original instruction that signified the end of
                        the region of inserted instructions. */
                    regionList.push_back(*instIter);
                } else {
                    //if the list is empty we add the original instruction.
                    //this is done so if it is just before a region it will be included in it. 
                    regionList.push_back(*instIter);
                }
                /*  Check if the instruction that is potentially ahead of a region
                    influences if the accumulator should be preserved or not. */
                switch(decodedMips.kind) {
                    case mips_div:
                    case mips_divu:
                    case mips_mult:
                    case mips_multu: {
                        /*  The instruction writes to acc so if it is ahead of an
                            transformed region acc should not be preserved. */
                        preserveAcc = false;
                        break;
                    }
                    default: {
                        /*  If we encounter a original instruction again that does
                            not write to acc we save the acc again. */
                        preserveAcc = true;
                    }
                }
            }
        }
    }
    /* Check if the instruction block ended with inserted instructions */
    if (regionList.empty() == false) {
        /* There is a region to perform allocation on, since the list is not empty. */ 
        regionAllocation(&regionList, preserveAcc); 
        /* Copy over the regionList */
        transformedInstructionVector.insert(transformedInstructionVector.end(),
            regionList.begin(), regionList.end());
    }
    /* Swap the transformedinstructionvector with the one in the basic block */
    instructionVector.swap(transformedInstructionVector);
}

/*  Transforms a region of inserted instructions so they have real registers */
void naiveHandler::regionAllocation(std::list<SgAsmStatement*>* regionList, bool preserveAcc) {
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
    
    /* if the accumulator register is used then add save and load instructions.
        If the preserveAcc is false then we do not preserve the acc.
        An additional condition is that we only add these instructions if
        there has actually been an region transformation, that can be 
        determined by checking if symbolicToHard is empty. */
    if (usesAcc && preserveAcc && (false == symbolicToHard.empty())) {
        /* Save the accumulator register */
        mipsRegisterName moveReg = symbolicToHard.begin()->second;
        saveAccumulator(regionList, moveReg);
    }
    
    // iterate through the symbolic to hard map and push to stack.
    for(std::map<unsigned, mipsRegisterName>::reverse_iterator symIter = symbolicToHard.rbegin();
        symIter != symbolicToHard.rend(); ++symIter) {
        /* Store instruction */
        SgAsmMipsInstruction* mipsStore = buildLoadOrStoreInstruction(mips_sw, symIter->second);
        /* insert the instruction */
        regionList->push_front(mipsStore);

        /* Load instruction */
        SgAsmMipsInstruction* mipsLoad = buildLoadOrStoreInstruction(mips_lw, symIter->second); 
        /* increment the offset */
        offset += 4;
        /* insert the instruction */
        regionList->push_back(mipsLoad);
    }
}

/* help functions to build load/store instructions */
SgAsmMipsInstruction* naiveHandler::buildLoadOrStoreInstruction(MipsInstructionKind kind, mipsRegisterName regname) {
        /* Stack pointer register that can be used */
        registerStruct spStruct;
        spStruct.regName = sp;
        /* a destination register or source register */
        registerStruct destinationOrSource;
        destinationOrSource.regName = regname;
        /* Create instruction struct accordingly then build instruction */
        instructionStruct loadstoreStruct;

        if (mips_lw == kind) {
            /* Specific setting for load instructions */
            loadstoreStruct.kind = mips_lw;
            loadstoreStruct.mnemonic = "lw";
            loadstoreStruct.format = getInstructionFormat(mips_lw);
            // set the destination register that is being restored.
            loadstoreStruct.destinationRegisters.push_back(destinationOrSource);
            // set the sp in the source register
            loadstoreStruct.sourceRegisters.push_back(spStruct);
        } else if (mips_sw == kind) {
            /*  specific settings for the store instructions. */
            loadstoreStruct.kind = mips_sw;
            loadstoreStruct.mnemonic = "sw";
            loadstoreStruct.format = getInstructionFormat(mips_sw);;
            // set sp in the source register
            loadstoreStruct.sourceRegisters.push_back(spStruct);
            // set the source register, which is being saved
            loadstoreStruct.sourceRegisters.push_back(destinationOrSource);
        } else {
            /* fail if a incorrect kind is supplied */
            ASSERT_not_reachable("Invalid kind supplied for load/store build function");
        }
        // set the data size and sign bits.
        loadstoreStruct.memoryReferenceSize = 32;
        loadstoreStruct.significantBits = 32;
        loadstoreStruct.isSignedMemory = true;
        //set the constant of the instruction. then increment it.
        loadstoreStruct.instructionConstant = offset;
        //build load instruction
        SgAsmMipsInstruction* mipsLoadOrStore = buildInstruction(&loadstoreStruct);
        //return the pointe to the mips instruction
        return mipsLoadOrStore;
}

/*  This function adds the necessary instructions to save and restore the
    accumulator register */
void naiveHandler::saveAccumulator(std::list<SgAsmStatement*>* regionList, mipsRegisterName tempReg) {
    //set the destination register which is one from symbolicToHard, can be used with both instructions.
    registerStruct moveReg;
    moveReg.regName = tempReg;
    
    /* low register store instruction */
    SgAsmMipsInstruction* mipsStoreLow = buildLoadOrStoreInstruction(mips_sw, tempReg);
    /* insert the instruction in the beginning of the region */
    regionList->push_front(mipsStoreLow);

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
    SgAsmMipsInstruction* mipsLoadLow = buildLoadOrStoreInstruction(mips_lw, tempReg);
    /* increase the offset */
    offset += 4;
    /* insert the instruction in the beginning of the region */
    regionList->push_back(mipsLoadLow);

    /* move to low instruction */
    instructionStruct mtlo;
    mtlo.kind = mips_mtlo;
    mtlo.mnemonic = "mtlo";
    mtlo.format = getInstructionFormat(mips_mtlo);
    //set the register as a destination register.
    mtlo.sourceRegisters.push_back(moveReg);
    //build instruction and insert.
    SgAsmMipsInstruction* mipsMtlo = buildInstruction(&mtlo);
    regionList->push_back(mipsMtlo);


    /* High register load and store instructions */
    /* store instruction for high register */
    SgAsmMipsInstruction* mipsStoreHigh = buildLoadOrStoreInstruction(mips_sw, tempReg);
    /* insert the instruction in the beginning of the region */
    regionList->push_front(mipsStoreHigh);

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

    /* load instruction for high register */
    SgAsmMipsInstruction* mipsLoadHigh = buildLoadOrStoreInstruction(mips_lw, tempReg);
    /* increment the offset */
    offset += 4;
    /* insert the instruction in the beginning of the region */
    regionList->push_back(mipsLoadHigh);
    
    /* move instruction for high register */
    instructionStruct mthi;
    mthi.kind = mips_mthi;
    mthi.mnemonic = "mthi";
    mthi.format = getInstructionFormat(mips_mthi);
    //set the register as a destination register.
    mthi.sourceRegisters.push_back(moveReg);
    //build instruction and insert.
    SgAsmMipsInstruction* mipsMthi = buildInstruction(&mthi);
    regionList->push_back(mipsMthi);
}


/*  Initalize the register set used during a region allocation */
void naiveHandler::initHardRegisters() {
    /*  Set stack offset count to zero */
    usedHardRegs = 0;
    /*  Clear the set */
    hardRegisters.clear();
    /*  Add temporary registers to the set */
    hardRegisters.insert(t0);
    hardRegisters.insert(t1);
    hardRegisters.insert(t2);
    hardRegisters.insert(t3);
    hardRegisters.insert(t4);
    hardRegisters.insert(t5);
    hardRegisters.insert(t6);
    hardRegisters.insert(t7);
    hardRegisters.insert(t8);
    hardRegisters.insert(t9);
    /*  Function variable registers. */
    hardRegisters.insert(s0);
    hardRegisters.insert(s1);
    hardRegisters.insert(s2);
    hardRegisters.insert(s3);
    hardRegisters.insert(s4);
    hardRegisters.insert(s5);
    hardRegisters.insert(s6);
    hardRegisters.insert(s7);
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
        ASSERT_not_reachable("Naivetransformer: Out of registers for transformation.");
    }
    return realReg;
}

/*  Adjust the size of the stack. Either activation record instructions are changed
    or they are created and inserted. */
//TODO this function relies on the old methods of modifying the stack
//TODO need to update them. 
void naiveHandler::modifyStack() {
    //TODO I have sets of activation and deactivation instructions. they
    //TODO can be checked first, if they are present then modify them.
    /*  Get the activation set and check if it contains activation instructions.
        If there are then modify them, otherwise create instructions. */
    std::set<SgAsmMipsInstruction*>* activationSet = cfgContainer->getActivationInstructions();
    /*  Check if the set has activation instructions. */
    if (0 < activationSet->size()) {
        /*  There are activation instructions. modify them. */
        for(std::set<SgAsmMipsInstruction*>::iterator actIter = activationSet->begin();
            actIter != activationSet->end(); ++actIter) {
            /*  Retrieve the instructions operand list. */
            SgAsmExpressionPtrList& allocOperands = (*actIter)->get_operandList()->get_operands();
            /*  Go through the operands, find the constant, modify it. */
            for(SgAsmExpressionPtrList::iterator iter = allocOperands.begin();
                iter != allocOperands.end(); ++iter) {
                /*  Find the constant in the instruction */
                if (V_SgAsmIntegerValueExpression == (*iter)->variantT()) {
                    /*  Get the value of it and adjust it for the new stack */
                    SgAsmIntegerValueExpression* valConst = isSgAsmIntegerValueExpression(*iter);
                    /* Get the current constant value */
                    uint64_t  constant = valConst->get_absoluteValue();
                    /*  adjust the value to reflect the new stack size, increase the
                        subtraction stack value.    */
                    constant -= (maximumSymbolicsUsed * 4);
                    /* set the new value */
                    valConst->set_absoluteValue(constant);
                }
            }
        }
    } else {
        /*  There are no activation instructions. Create my own. */
        /*  Instruction struct. */
        instructionStruct naiveActivation;
        /*  Register struct for the SP register. */
        registerStruct regSP;
        regSP.regName = sp;
        /*  Set kind, mnemonic, format. */
        naiveActivation.kind = mips_addiu;
        naiveActivation.mnemonic = "addiu";
        naiveActivation.format = getInstructionFormat(mips_addiu);
        /*  set the constant, which is the stack space allocated. */
        naiveActivation.instructionConstant = -(maximumSymbolicsUsed * 4);
        naiveActivation.significantBits = 32;
        /*  Set SP as both source and destination register. */
        naiveActivation.sourceRegisters.push_back(regSP);
        naiveActivation.destinationRegisters.push_back(regSP);
        /*  Build instruction. */
        SgAsmMipsInstruction* mipsAlloc = buildInstruction(&naiveActivation);
        /*  Get entry block. */
        SgAsmBlock* eb = cfgContainer->getEntryBlock();
        /*  Get statement list. */
        SgAsmStatementPtrList& stmtList = eb->get_statementList();
        /*  Insert activation instruction in the beginning of the statement list. */
        stmtList.insert(stmtList.begin(), mipsAlloc);
    }

    /*  Check the deactivation set and to the same as for the
        activation instructions. */
    std::set<SgAsmMipsInstruction*>* deactivationSet = cfgContainer->getDeactivationInstructions();
    /*  Check the sets size. */
    if (0 < deactivationSet->size()) {
        /*  There are deactivation instructions, modify them. */
        for(std::set<SgAsmMipsInstruction*>::iterator deIter = deactivationSet->begin();
            deIter != deactivationSet->end(); ++deIter) {
            /*  Get the instructions operand list. */
            SgAsmExpressionPtrList& deallocOperands = (*deIter)->get_operandList()->get_operands();
            /*  Go through the operands, find the constant, modify it. */
            for(SgAsmExpressionPtrList::iterator iter = deallocOperands.begin();
                iter != deallocOperands.end(); ++iter) {
                /*  Find the constant in the instruction */
                if (V_SgAsmIntegerValueExpression == (*iter)->variantT()) {
                    /*  Get the value of it and adjust it for the new stack */
                    SgAsmIntegerValueExpression* valConst = isSgAsmIntegerValueExpression(*iter);
                    /* Get the current constant value */
                    uint64_t  constant = valConst->get_absoluteValue();
                    /*  adjust the value to reflect the new stack size, increase the
                        added value stack value.    */
                    constant += (maximumSymbolicsUsed * 4);
                    /* set the new value */
                    valConst->set_absoluteValue(constant);
                }
            }
        }
    } else {
        /*  There are not deactivation instructions so create and insert into the
            exit blocks. */
        std::set<SgAsmBlock*>* exitBlockSet = cfgContainer->getExitBlocks();
        /*  Iterate over the blocks and add a deactivation instructions. */
        for(std::set<SgAsmBlock*>::iterator ebIter = exitBlockSet->begin();
            ebIter != exitBlockSet->end(); ++ebIter) {
            /*  Instruction struct. */
            instructionStruct naiveDeactivation;
            /*  Register struct for the SP register. */
            registerStruct regSP;
            regSP.regName = sp;
            /*  Set kind, mnemonic, format. */
            naiveDeactivation.kind = mips_addiu;
            naiveDeactivation.mnemonic = "addiu";
            naiveDeactivation.format = getInstructionFormat(mips_addiu);
            /*  set the constant, which is the stack space allocated. */
            naiveDeactivation.instructionConstant = (maximumSymbolicsUsed * 4);
            naiveDeactivation.significantBits = 32;
            /*  Set SP as both source and destination register. */
            naiveDeactivation.sourceRegisters.push_back(regSP);
            naiveDeactivation.destinationRegisters.push_back(regSP);
            /*  Build instruction. */
            SgAsmMipsInstruction* mipsDealloc = buildInstruction(&naiveDeactivation);
            /*  Get statement list. */
            SgAsmStatementPtrList& stmtList = (*ebIter)->get_statementList();
            /*  Insert the deactivation instruction second to last is the list. */
            stmtList.insert(--stmtList.end(), mipsDealloc);
        }
    }
}

/* Find the maximum amount of used symbolic registers used at the same time.
    Also check if an original instruction is using the acc register. */
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


