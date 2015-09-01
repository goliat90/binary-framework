/* Source code for the framework linear scan */



/* includes */
#include "linearScan.hpp"


/*  Constructor */
linearScanHandler::linearScanHandler(CFGhandler* passedCfgObject){
    /*  Set Default values */
    debuging = false;
    /*  reset stack offset. */
    stackOffset = 0;
    /*  reset spill counter */
    maxSpillOffset = 0;
    /*  Save the pointer to the cfg handler */
    cfgHandlerPtr = passedCfgObject;
    /*  Create the live variable analysis object */
    liveRangeHandler = new liveVariableAnalysisHandler(cfgHandlerPtr);
    /*  Initialize the register pool. */
    initializeRegisterPool();
}

/*  Initialize function for the register pool. Current implementation only uses
    the temporary registers. If other are to be used the live-range analysis needs to
    be fixed to include the semantics of those registers, such as they are return
    value registers or arguments for function. */
void linearScanHandler::initializeRegisterPool() {
    registerPool.clear();
    /*  Initialize the freeRegister set */
    /*  t0-t7 */
    registerPool.push_front(t0);
    registerPool.push_front(t1);
    registerPool.push_front(t2);
    registerPool.push_front(t3);
    registerPool.push_front(t4);
    registerPool.push_front(t5);
    registerPool.push_front(t6);
    registerPool.push_front(t7);
    /*  t8-t9 */
    //registerPool.push_front(t8);
    //registerPool.push_front(t9);
}


/*  Activate or deactivate debuging printout*/
void linearScanHandler::selectDebuging(bool mode) {
    debuging = mode;
    /*  pass it to the liverange analysis */
    liveRangeHandler->setDebug(mode);
}

/*  Applies the linear scan */
void linearScanHandler::applyLinearScan() {
    /*  Go through the cfg and replace the register that linear scan will use. */
    if (debuging) {
        std::cout << "Replacing physical registers used by linear scan." << std::endl;
    }
    replaceHardRegisters();

    //TODO here i need to check if any of the original instructions uses the accumulator.
    //if so i have to save it when needed. */
    if (debuging) {
        std::cout << "Checking for use of accumulator in original instructions." << std::endl;
    }
    checkAccumulatorAndFix();

    /*  Get live-range analysis done before performing register allocation. */
    if (debuging) {
        std::cout << "Starting the live-range analysis." << std::endl;
    }
    liveRangeHandler->performLiveRangeAnalysis();

    /*  All preparations for register allocation is done, perform linear scan allocation. */
    if (debuging) {
        std::cout << "Starting Linear Scan register allocation." << std::endl;
    }
    linearScanAllocation();

    /*  Replace all symbolic registers with real register and
        insert instructions for spilled registers. */
    if (debuging) {
        std::cout << "Replacing symbolic registers." << std::endl; 
    }
    replaceSymbolicRegisters();

    /*  After linear scan has been performed modify the stack. If there is no
        stack then create stack instructions. */
    if (debuging) {
        std::cout << "Modifying stack." << std::endl;
    }
    linearStackModification();
}

/*  Replace the registers used by linear scan with symbolic names.
    Each physical register will be replaced with one symbolic register that
    will be used in the whole CFG. This could be considered a greedy method
    since it will result in long live ranges for these registers. */
void linearScanHandler::replaceHardRegisters() {
    /*  Reset the register pool. */
    initializeRegisterPool();
    /*  mapping between physical register and symbolic name */
    std::map<mipsRegisterName, registerStruct> physicalToSymbolic;
    /*  CFG pointer */
    CFG* functionCFG = cfgHandlerPtr->getFunctionCFG();
    /*  Iterate through the vertices */
    for(std::pair<CFGVIter, CFGVIter> vertPair = vertices(*functionCFG);
        vertPair.first != vertPair.second; ++vertPair.first) {
        /*  Get the block pointer */
        SgAsmBlock* block = get(boost::vertex_name, *functionCFG, *vertPair.first);
        /*  Retrieve the statement list */
        SgAsmStatementPtrList& stmtList = block->get_statementList();
        /*  Go through the statement list and look for the specific registers */
        for(SgAsmStatementPtrList::iterator stmtIter = stmtList.begin();
            stmtIter != stmtList.end(); ++stmtIter) {
            /*  Verify that the statement is a mips instruction */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /*  Cast to mips instruction pointer */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                /*  Get the operand list and check for instructions. */
                SgAsmExpressionPtrList& opList = mips->get_operandList()->get_operands(); 
                /*  Go through the operands */
                for(int i = 0; i < opList.size(); i++) {
                    /*  Get pointer to the expression */
                    SgAsmExpression* expr = opList.at(i);
                    /*  Check if the expression is a register */
                    if (V_SgAsmDirectRegisterExpression == expr->variantT()) {
                        /*  Decode the register */
                        registerStruct reg = decodeRegister(expr);
                        /*  Determine if the register should be replaced or not.
                            Also check if it has been replaced already. */
                        if (registerPool.end() != std::find(registerPool.begin(), registerPool.end(), reg.regName)) {
                            /*  The register should be replaced, check if it has a symbolic or not */
                            if (0 == physicalToSymbolic.count(reg.regName)) {
                                /*  First time it has been encountered, generate a symbolic */
                                registerStruct symReg = generateSymbolicRegister();
                                /*  Debug printout */
                                if (debuging) {
                                    std::cout << "Replaced a register with sym_" << std::dec
                                                << symReg.symbolicNumber << " saved to map." << std::endl;
                                }
                                /*  Insert values into the map */
                                physicalToSymbolic.insert(std::pair<mipsRegisterName, registerStruct>(reg.regName, symReg));
                                /*  Build the register expression */
                                SgAsmDirectRegisterExpression* DRE = buildRegister(symReg);
                                /*  Replace the register expression with the new expression. */
                                opList.at(i) = DRE;
                            } else {
                                /*  The register should be replaced, it has been replaced before. */
                                registerStruct usedSymReg = physicalToSymbolic.find(reg.regName)->second;
                                /*  Debug printout */
                                if (debuging) {
                                    std::cout << "Replaced a register with sym_" << std::dec
                                                << usedSymReg.symbolicNumber << std::endl;
                                }
                                /*  Get the register expression */
                                SgAsmDirectRegisterExpression* usedDRE = buildRegister(usedSymReg);
                                /*  Replace the expression. */
                                opList.at(i) = usedDRE;
                            }
                        }
                    }
                }
            }
        }
    }

    if (debuging) {
        for(std::pair<CFGVIter, CFGVIter> vertPair = vertices(*functionCFG);
            vertPair.first != vertPair.second; ++vertPair.first) {
            /*  Get the block pointer */
            SgAsmBlock* block = get(boost::vertex_name, *functionCFG, *vertPair.first);
            /*  Print the basic block if debuging */
            printBasicBlockInstructions(block);
        }
    }
}


/*  Checks the original instructions if there is an instruction that
    uses the accumulator. */
void linearScanHandler::checkAccumulatorAndFix() {
    /*  Bool set if a original instruction uses acc. */
    bool originalUsesAcc = false;
    /*  CFG pointer */
    CFG* functionCFG = cfgHandlerPtr->getFunctionCFG();
    /*  Iterate through the vertices */
    for(std::pair<CFGVIter, CFGVIter> vertPair = vertices(*functionCFG);
        vertPair.first != vertPair.second; ++vertPair.first) {
        /*  Get the block pointer */
        SgAsmBlock* block = get(boost::vertex_name, *functionCFG, *vertPair.first);
        /*  Retrieve the statement list */
        SgAsmStatementPtrList& stmtList = block->get_statementList();
        /*  Go through the statement list and look at original instructions (has address)
            and see if they are a type that uses the accumulator register. */
        for(SgAsmStatementPtrList::iterator stmtIter = stmtList.begin();
            stmtIter != stmtList.end(); ++stmtIter) {
            /*  Verify that the statement is a mips instruction */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /*  Cast to mips instruction pointer */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                /*  Check if it is a original instruction. */
                if (0 != mips->get_address()) {
                    /*  Check the instruction kind and see if the instruction
                        writes to accumulator registers. */
                    switch(mips->get_kind()) {
                        case mips_div:
                        case mips_divu:
                        case mips_mult:
                        case mips_multu:{
                            /*  These instructions writes to accumulator register. */
                            originalUsesAcc = true;
                            if (debuging) {
                                std::cout << "An original instruction writes to accumulator register." << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }

    /*  If a original instruction uses the accumulator register i need
        to save accumulator before using it in inserted instructions and then restore it. */
    if (true == originalUsesAcc) {
        /*  Iterate through the vertices */
        for(std::pair<CFGVIter, CFGVIter> vertPair = vertices(*functionCFG);
            vertPair.first != vertPair.second; ++vertPair.first) {
            /*  Get the block pointer */
            SgAsmBlock* block = get(boost::vertex_name, *functionCFG, *vertPair.first);
            /*  Retrieve the statement list */
            SgAsmStatementPtrList& stmtList = block->get_statementList();
            /*  Shadow statement list. */
            std::list<SgAsmStatement*> shadowList;
            /*  Lists to temporary store load, stores and moves. */
            SgAsmStatementPtrList beforeInserts;
            SgAsmStatementPtrList afterInserts;
            /*  First instruction in region and a iterator for it. */
            bool firstInRegion = true;
            std::list<SgAsmStatement*>::iterator firstInst;
            /*  Bool used to prevent adding several preservation instructions to a region.
                It is also used to disable adding preservation if the instruction just
                before the region writes to acc. */
            bool preserveAcc = true;
            /*  List of instruction for a region of inserted instructions. */
            std::list<SgAsmStatement*> regionList;
            /*  Go through the statement list and look at original instructions (has address)
                and see if they are a type that uses the accumulator register. */
            for(SgAsmStatementPtrList::iterator stmtIter = stmtList.begin();
                stmtIter != stmtList.end(); ++stmtIter) {
                /*  Verify that the statement is a mips instruction */
                if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                    /*  Cast to mips instruction pointer */
                    SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                    /*  Check if it is an inserted instruction. */
                    if (0 == mips->get_address()) {
                        /*  Check the instruction kind and see if the instruction
                            writes to accumulator registers. */
                        switch(mips->get_kind()) {
                            case mips_div:
                            case mips_divu:
                            case mips_mult:
                            case mips_multu:{
                                if (true == preserveAcc) { 
                                    /*  These instructions writes to accumulator register. */
                                    //TODO set a bool here to say that we need to save acc in this region?

                                    //TODO move lo to a register.
                                    instructionStruct mflo;
                                    mflo.kind = mips_mflo;
                                    mflo.mnemonic = "mflo1";
                                    mflo.format = getInstructionFormat(mips_mflo);
                                    /*  Get a destination symbolic. */
                                    registerStruct fromLoReg = generateSymbolicRegister();
                                    mflo.destinationRegisters.push_back(fromLoReg);
                                    /*  Build instruction and save it. */
                                    SgAsmMipsInstruction* mipsmflo = buildInstruction(&mflo);
                                    beforeInserts.push_back(mipsmflo);
                                    //TODO store it on the stack.
                                    SgAsmMipsInstruction* mipsLoStore = buildLoadOrStoreSpillInstruction(mips_sw, fromLoReg, 0);
                                    beforeInserts.push_back(mipsLoStore);

                                    //TODO move hi to a register.
                                    instructionStruct mfhi;
                                    mfhi.kind = mips_mfhi;
                                    mfhi.mnemonic = "mfhi1";
                                    mfhi.format = getInstructionFormat(mips_mfhi);
                                    /*  Get a destination symbolic. */
                                    registerStruct fromHiReg = generateSymbolicRegister();
                                    mfhi.destinationRegisters.push_back(fromHiReg);
                                    /*  Build instruction and save it. */
                                    SgAsmMipsInstruction* mipsmfhi = buildInstruction(&mfhi);
                                    beforeInserts.push_back(mipsmfhi);
                                    //TODO store it on the stack.
                                    SgAsmMipsInstruction* mipsHiStore = buildLoadOrStoreSpillInstruction(mips_sw, fromHiReg, 4);
                                    beforeInserts.push_back(mipsHiStore);

                                    //TODO prepare a load instruction to insert after.
                                    registerStruct toLoReg = generateSymbolicRegister();
                                    SgAsmMipsInstruction* mipsLoLoad = buildLoadOrStoreSpillInstruction(mips_lw, toLoReg, 0);
                                    afterInserts.push_back(mipsLoLoad);
                                    //TODO prepare a move register to acc instruction.
                                    instructionStruct mtlo;
                                    mtlo.kind = mips_mtlo;
                                    mtlo.mnemonic = "mtlo2";
                                    mtlo.format = getInstructionFormat(mips_mtlo);
                                    /*  Get a destination symbolic. */
                                    mtlo.sourceRegisters.push_back(toLoReg);
                                    /*  Build instruction and save it. */
                                    SgAsmMipsInstruction* mipsmtlo = buildInstruction(&mtlo);
                                    afterInserts.push_back(mipsmtlo);
                                    //TODO prepare a load instruction to insert after.
                                    registerStruct toHiReg = generateSymbolicRegister();
                                    SgAsmMipsInstruction* mipsHiLoad = buildLoadOrStoreSpillInstruction(mips_lw, toHiReg, 4);
                                    afterInserts.push_back(mipsHiLoad);
                                    //TODO prepare a move register to acc instruction.
                                    instructionStruct mthi;
                                    mthi.kind = mips_mthi;
                                    mthi.mnemonic = "mthi2";
                                    mthi.format = getInstructionFormat(mips_mthi);
                                    mthi.sourceRegisters.push_back(toHiReg);
                                    SgAsmMipsInstruction* mipsmthi = buildInstruction(&mthi);
                                    afterInserts.push_back(mipsmthi);
                                    //the instructions restoring acc need to be placed in a list for insertion
                                    //when the next original instruction is found.
                                    /*  Set to false to prevent making duplicate preservation instructions. */
                                    preserveAcc = false;
                                }
                            }
                        }
                        /*  It is an instruction that does not use acc. Just store it. */
                        //If it is the first inserted in a region then save an iterator to it. 
                        //Im refering to an iterator in the shadow list.
                        //could be used to insert move and stores to the beginning of the region in the shadow list.
                        if (true == firstInRegion) {
                            firstInRegion = false;
                            firstInst = shadowList.insert(shadowList.end(), *stmtIter);
                        } else {
                            /*  not first in region but needs to be saved. */
                            shadowList.push_back(*stmtIter);
                        }
                    } else {
                        /*  The address is not 0 so it is an original instruction. */
                        /*  Check if the before and after region lists are empty or not */
                        if (!beforeInserts.empty()) {
                            /*  Insert instructions at the start of the region using the
                                saved iterator. */
                            shadowList.insert(firstInst, beforeInserts.begin(), beforeInserts.end());
                            beforeInserts.clear();
                        }
                        if (!afterInserts.empty()) {
                            /*  Insert instructions at the end of the region. */
                            shadowList.insert(shadowList.end(), afterInserts.begin(), afterInserts.end());
                            afterInserts.clear();
                        }
                        //TODO here i should do a check on the original instruction if it
                        //TODO writes to acc, if so then we should skip the preservation of acc in the region.
                        /*  Check if this original instruction writes to acc, if so then
                            acc will not be preserved. */
                        switch(mips->get_kind()) {
                            case mips_div:
                            case mips_divu:
                            case mips_mult:
                            case mips_multu:{
                                /*  The instruction writes to acc. Skip preserving it. */
                                preserveAcc = false;
                                break;
                            }
                            default: {
                                preserveAcc = true;
                            }
                        }
                        /*  Save this original instruction. */
                        shadowList.push_back(*stmtIter);
                        /*  Set first in region bool to true again. */
                        firstInRegion = true;
                    }
                } else {
                    /*  It is not a instruction but needs to be saved. */
                    shadowList.push_back(*stmtIter);
                }
            }
            /*  Check here if the before and after insert lists are not empty.
                If they are not there are instructions to be inserted. */
            if (!beforeInserts.empty()) {
                /*  Insert instructions at the start of the region using the
                    saved iterator. */
                shadowList.insert(firstInst, beforeInserts.begin(), beforeInserts.end());
                beforeInserts.clear();
            }
            if (!afterInserts.empty()) {
                /*  Insert instructions at the end of the region. */
                shadowList.insert(shadowList.end(), afterInserts.begin(), afterInserts.end());
                afterInserts.clear();
            }

            /*  The statement list has been iterated and the shadowList needs
                to be exhanged with the statement list. */
                //TODO clear the statement list? then do insert?
                SgAsmStatementPtrList temp(shadowList.begin(), shadowList.end());
                stmtList.swap(temp);
        }
        /*  Increase the stack value so we dont overwrite the acc values. */
        stackOffset += 8;
    }
}


/*  Algorithms for linear scan. */ 
void linearScanHandler::linearScanAllocation() {
    /*  Setting up variables and initial values. */
    /*  Clearing active. */
    activeMap.clear();
    int registerPoolSize = registerPool.size();
    if (debuging) {
        std::cout << "registerPool: " << registerPoolSize << std::endl;
    }
    /*  Retrieving the Pointers for the start and endpoint maps. */
    startPointMap = liveRangeHandler->getStartPoints();
    endPointMap = liveRangeHandler->getEndPoints();

    /*  Iterate through the intervals, lowest start point to highest. */
    for(intervalMap::left_iterator intervalIter = startPointMap->left.begin();
        intervalIter != startPointMap->left.end(); ++intervalIter) {
        /*  Expire old intervals at the current point. */
        expireOldIntervals(intervalIter->first);
        /*  Check if all registers are in use, if so spill an interval.
            Otherwise allocate register for the interval. */
        if (debuging) {
            std::cout << "Symbolics active" << std::endl;
            for(intervalMap::left_iterator lIter = activeMap.left.begin();
                lIter != activeMap.left.end(); ++lIter) {
                std::cout << "Symbolic: " << lIter->second << " active until " << lIter->first << std::endl;
            }
        }

        if (registerPoolSize == activeMap.size()) {
            //SPILL interval.
            if (debuging) {
                std::cout << "Spilling at point: " << intervalIter->first << std::endl;
            }
            spillAtInterval(intervalIter);
        } else {
            if (debuging) {
                std::cout << "Allocating a register." << std::endl;
                std::cout << "activeMap size = " << activeMap.size() << std::endl;
            }
            /*  Take a register from the pool and remove it form the pool. */
            mipsRegisterName intervalReg = registerPool.front();
            registerPool.pop_front();
            /*  Save the interval and its register. */
            allocationMap.insert(std::pair<unsigned, mipsRegisterName>(intervalIter->second, intervalReg));
            if (debuging) {
                std::cout << "symbolic: " << intervalIter->second << " allocated to register "
                    << getRegisterString(intervalReg) << std::endl;
            }
            /*  Add the interval to the active list. Get the end point for the interval. */
            intervalMap::right_iterator intervalEndPoint = endPointMap->right.find(intervalIter->second);
            /*  Insert the values. */
            activeMap.insert(intervalMap::value_type(intervalEndPoint->second, intervalEndPoint->first));
        }
    }

    /*  Debug printout */
    if (debuging) {
        std::cout << "---------- Allocation ----------" << std::endl;
        /*  Printout the allocation map. */
        for(std::map<unsigned, mipsRegisterName>::iterator allocIter = allocationMap.begin();
            allocIter != allocationMap.end(); ++allocIter) {
            /*  Print the entry. */
            std::cout << "Symbolic: " << allocIter->first << " Mapped to register "
                << getRegisterString(allocIter->second) << std::endl;
        }
        std::cout << "---------- Allocation End ----------" << std::endl << std::endl;

        /*  Print the spill map. */
        std::cout << "---------- Spills ----------" << std::endl;
        for(std::map<unsigned, uint64_t>::iterator spillIter = spillMap.begin();
            spillIter != spillMap.end(); ++spillIter) {
            /*  Print the map entry. */
            std::cout << "Symbolic: " << std::dec << spillIter->first << " spilled. Offest: "
                << std::hex << spillIter->second << std::endl;
        }
        std::cout << "---------- Spills End ----------" << std::endl << std::endl;
    }
}


void linearScanHandler::expireOldIntervals(int startPoint) {
    /*  Temporary storage for the intervals being removed. */
    std::set<unsigned> oldIntervals;
    /*  debug printout */
    if (debuging) {
        std::cout << "Searching for expired intervals at point: " << startPoint << std::endl;
    }
    /*  Iterate through the active list and remove all
        intervals that have expired. */
    for(intervalMap::right_iterator activeIter = activeMap.right.begin();
        activeIter != activeMap.right.end(); ++activeIter) {
        /*  Check if the interval has expired or not. */
        if ((activeIter->second) >= startPoint) {
            continue;
        } else {
            /*  Save the interval so it can be removed after the iteration. */
            oldIntervals.insert(activeIter->first);
            if (debuging) {
                std::cout << "Interval symbolic: " << activeIter->first
                    << " with end point: " << activeIter->second 
                    << " has expired. " << std::endl;
            }
            /*  Get the register that the interval has used. */
            mipsRegisterName intervalReg = allocationMap.find(activeIter->first)->second;
            if (debuging) {
                std::cout << "register: " << getRegisterString(intervalReg)
                << " returned to pool." << std::endl;
            }
            /*  Return the register to the pool of free registers. */
            registerPool.push_back(intervalReg);
        }
    }

    /*  Remove the intervals that have expired. */
    for(std::set<unsigned>::iterator removeIter = oldIntervals.begin();
        removeIter != oldIntervals.end(); ++removeIter) {
        /* Call erase on the map. */
        activeMap.right.erase(*removeIter);
    }
}

/*  spill interval function. newInterval, first = endPoint, second = symbolic. */
void linearScanHandler::spillAtInterval(intervalMap::left_iterator newInterval) {
    /*  Get the last interval in active. Endpoint and symbolic.
        first = endpoint, second = symbolic. */
    intervalMap::left_reverse_iterator lastActiveInterval = activeMap.left.rbegin();
    /*  Get the end point of the new interval. 
        first = symbolic, second = endpoint */
    intervalMap::right_iterator newIntervalEnd = endPointMap->right.find(newInterval->second);
    /*  Check if the last interval in active should be spilled or the new interval. */
    if ((lastActiveInterval->first) > (newIntervalEnd->second)) {
        if (debuging) {
            std::cout << "Spilling last active interval: " << lastActiveInterval->second  
                << " since its end point (" << lastActiveInterval->first
                << ") is further away than new end point " << newIntervalEnd->second 
                << " belonging to interval " << newIntervalEnd->first << std::endl;
        }
        /*  Get the register used by the interval being spilled.  */
        mipsRegisterName intervalReg = allocationMap.find(lastActiveInterval->second)->second;
        if (debuging) {
            std::cout << "symbolic: " << lastActiveInterval->second << " used register "
                << getRegisterString(intervalReg) << std::endl;
        }
        /*  Remove the interval being spilled from the allocationmap. */
        allocationMap.erase(lastActiveInterval->second);
        /*  Add the last active interval which is being spilled to the spill map. */
        spillMap.insert(std::pair<unsigned, uint64_t>(lastActiveInterval->second, stackOffset));
        /*  Increment stack offset. */
        stackOffset += 4;

        /*  Insert the new interval into the allocation map with
            the register that the last active interval was given. */
        allocationMap.insert(std::pair<unsigned, mipsRegisterName>(newInterval->second, intervalReg));
        /*  Remove the last active interval from the active list. */
        activeMap.right.erase(lastActiveInterval->second);
        /*  Add the new interval to the active list. */
        activeMap.insert(intervalMap::value_type(newIntervalEnd->second, newIntervalEnd->first));
    } else {
        /*  The new interval has the furthest end point. Spill it to memory.
            Add it to the spill map and then increment the offset. */
        spillMap.insert(std::pair<unsigned, uint64_t>(newInterval->second, stackOffset));
        stackOffset += 4;
        /*  Debug printout. */
        if (debuging) {
            std::cout << "Spilling symbolic: " << newInterval->second << " immediateley." << std::endl << std::endl;
        }
    }
}


/*  Replaces all symbolic registers with real and fix
    load and store instructions for the spilled intervals. */
void linearScanHandler::replaceSymbolicRegisters() {
    /*  Print list. */
    initializeRegisterPool();

    if (debuging) {
        std::cout << "pool size:" << registerPool.size() << std::endl;
        for(std::list<mipsRegisterName>::iterator iter = registerPool.begin();
            iter != registerPool.end(); ++iter) {
            std::cout << getRegisterString(*iter) << std::endl;
        }
    }
    /*  Variables */
    CFG* functionCFG = cfgHandlerPtr->getFunctionCFG();
    /*  Iterate through the blocks and check each instruction for
        for symbolic registers and replace them. */
    for(std::pair<CFGVIter, CFGVIter> vIter = vertices(*functionCFG);
        vIter.first != vIter.second; ++vIter.first) {
        /*  Get the block pointer. */
        SgAsmBlock* basic = get(boost::vertex_name, *functionCFG, *vIter.first);
        /*  Get the statementlist. */
        SgAsmStatementPtrList& stmtList = basic->get_statementList();
        /*  Create a shadowlist that will be swapped with the statementlist. */
        SgAsmStatementPtrList shadowList;
        /*  Iterate through the statement list. */
        for(SgAsmStatementPtrList::iterator stmtIter = stmtList.begin();
            stmtIter != stmtList.end(); ++stmtIter) {
            /*  Check if the statement is a mips instruction. */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /*  Cast to mips instruction pointer. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction((*stmtIter));
                /*  Set to track registers used in the instruction. At least registers relevant
                    to linear scan. */
                std::set<mipsRegisterName> regsUsed;
                /*  Get the operand list from the instruction. */
                SgAsmExpressionPtrList& opList = mips->get_operandList()->get_operands();
                /*  Go through the operands and check if they are registers.
                    If they are then check if they are symbolic, i.e zero registers. */
                for(SgAsmExpressionPtrList::iterator opIter = opList.begin();
                    opIter != opList.end(); ++opIter) {
                    /*  Is it a register. */
                    if (V_SgAsmDirectRegisterExpression == (*opIter)->variantT()) {
                        /*  Cast to register exporession. */
                        SgAsmDirectRegisterExpression* regPtr = isSgAsmDirectRegisterExpression(*opIter);
                        /*  Decode register and check if it is symbolic. */
                        registerStruct regS = decodeRegister(regPtr);
                        if (symbolic_reg == regS.regName) {
                            /*  Check if the register is allocated. */
                            if (1 == allocationMap.count(regS.symbolicNumber)) {
                                /*  Register struct */
                                registerStruct newHardReg;
                                /*  The symbolic is allocated. Retrieve which register it is allocated. */
                                newHardReg.regName = allocationMap.find(regS.symbolicNumber)->second;
                                /*  Build the register expression. */
                                SgAsmDirectRegisterExpression* newRegExpr = buildRegister(newHardReg);
                                /*  Insert the new register expression into the operand list. */
                                (*opIter) = newRegExpr;
                                /*  Save the register to the set for later checking. */
                                regsUsed.insert(newHardReg.regName);
                            }
                        }
                    }
                }
                /*  Now all symbolic allocated have been given their registers in the instruction.
                    Next is to fix the spilled symbolics in the instruction. */

                /*  Map to track if a spilled has been given a register already.
                    Think when a spilled symbolic is source and destination. */
                std::map<unsigned, mipsRegisterName> spillRegs;
                /*  Lists of spill symbolics load and store instructions. */
                SgAsmStatementPtrList beforeSpillInst;
                SgAsmStatementPtrList afterSpillInst;
                /*  Spill offset. Added to the regular offset. */
                uint64_t spillOffset = 0;
                /*  Spill register name variable. */
                mipsRegisterName spillReg;
                /*  Decode the instruction. */
                instructionStruct decodedMips = decodeInstruction(mips);
                /*  Check the source registers. */
                for(std::vector<registerStruct>::iterator sIter = decodedMips.sourceRegisters.begin();
                    sIter != decodedMips.sourceRegisters.end(); ++sIter) {
                    /* Check if the register is symbolic. */
                    if (symbolic_reg == (*sIter).regName) {
                        /*  Check if the register is spilled. */
                        if (1 == spillMap.count((*sIter).symbolicNumber)) {
                            /*  The register was spilled. Need to free a register to use temporarily. */
                            /*  Check if the spilled symbolic already has a reg for this instruction. */
                            if (1 == spillRegs.count((*sIter).symbolicNumber)) {
                                /*  It has a register so use it again. */
                                spillReg = spillRegs.find((*sIter).symbolicNumber)->second;
                            } else {
                                /*  It did not have a register so get one. */
                                for(std::list<mipsRegisterName>::iterator rIter = registerPool.begin();
                                    rIter != registerPool.end(); ++rIter) {
                                    /*  Find the first register in the pool we know is not used in the instruction. */
                                    if (1 != regsUsed.count(*rIter)) {
                                        spillReg = *rIter;
                                        break;
                                    }
                                }
                            }
                            /*  Set it as a used register for the instruction. */
                            regsUsed.insert(spillReg);
                            /*  Map the register to the symbolic. */
                            spillRegs.insert(std::pair<unsigned, mipsRegisterName>((*sIter).symbolicNumber, spillReg));
                            /*  Create a store instruction to save the value of the register temporarily. */
                            /*  Struct for the spill register. */
                            registerStruct spillStruct;
                            spillStruct.regName = spillReg;
                            SgAsmMipsInstruction* store = buildLoadOrStoreSpillInstruction
                                (mips_sw, spillStruct, stackOffset+spillOffset);
                            beforeSpillInst.push_back(store);
                            /*  The value residing in the register is temporarily stored, now the spilled
                                value can be loaded and used in the instruction. Afterwards it is saved in memeory. */
                            /*  Get the offset for the spilled value. */
                            uint64_t memVariableOffset = spillMap.find((*sIter).symbolicNumber)->second;
                            /*  Build load and store instructions using the offset. */
                            SgAsmMipsInstruction* memVarLoad = buildLoadOrStoreSpillInstruction
                                (mips_lw, spillStruct, memVariableOffset);
                            beforeSpillInst.push_back(memVarLoad);

                            //TODO Is it really right to save the variable if it is only used as source?
                            //TODO it is not being changed unless it is also a destination.
                            //SgAsmMipsInstruction* memVarStore = buildLoadOrStoreSpillInstruction
                            //    (mips_sw, spillStruct, memVariableOffset);
                            //afterSpillInst.push_back(memVarStore);
                            
                            /*  Create a load instruction to retrieve the temporary stored value. */
                            SgAsmMipsInstruction* load = buildLoadOrStoreSpillInstruction
                                (mips_lw, spillStruct, stackOffset+spillOffset);
                            afterSpillInst.push_back(load);
                            /*  Increase the spill offset. */
                            spillOffset += 4;
                            /*  Return the register used as spill to the pool again. */
                            registerPool.push_back(spillReg);
                        } 
                    }
                }
                /*  Check destination registers. */
                for(std::vector<registerStruct>::iterator dIter = decodedMips.destinationRegisters.begin();
                    dIter != decodedMips.destinationRegisters.end(); ++dIter) {
                    /* Check if the register is symbolic. */
                    if (symbolic_reg == (*dIter).regName) {
                        /*  Check if the register is spilled. And if it has a register or not. */
                        if (1 == spillMap.count((*dIter).symbolicNumber)) {
                            /*  Check if the symbolic has a register or needs one. */
                            if (1 == spillRegs.count((*dIter).symbolicNumber)) {
                                /* It has a register, use it. */
                                spillReg = spillRegs.find((*dIter).symbolicNumber)->second;
                            } else {
                                /*  It did not have a register so get one. */
                                for(std::list<mipsRegisterName>::iterator rIter = registerPool.begin();
                                    rIter != registerPool.end(); ++rIter) {
                                    /*  Find the first register in the pool we know is not used in the instruction. */
                                    if (1 != regsUsed.count(*rIter)) {
                                        spillReg = *rIter;
                                        break;
                                    }
                                }
                            }
                            /*  Set it as a used register for the instruction. */
                            regsUsed.insert(spillReg);
                            /*  Map the register to the symbolic. */
                            spillRegs.insert(std::pair<unsigned, mipsRegisterName>((*dIter).symbolicNumber, spillReg));
                            /*  Create a store instruction to save the value of the register temporarily. */
                            registerStruct spillStruct;
                            spillStruct.regName = spillReg;
                            SgAsmMipsInstruction* store = buildLoadOrStoreSpillInstruction
                                (mips_sw, spillStruct, stackOffset+spillOffset);
                            beforeSpillInst.push_back(store);
                            /*  The value residing in the register is temporarily stored, now the spilled
                                value can be stored after it has recieved its new value. */
                            /*  Get the offset for the spilled value. */
                            uint64_t memVariableOffset = spillMap.find((*dIter).symbolicNumber)->second;
                            /*  Create a store instruction using the offset. */
                            SgAsmMipsInstruction* memVariableStore = buildLoadOrStoreSpillInstruction
                                (mips_sw, spillStruct, memVariableOffset);
                            afterSpillInst.push_back(memVariableStore);
                            /*  Create a load instruction to retrieve the temporary stored value. */
                            SgAsmMipsInstruction* load = buildLoadOrStoreSpillInstruction
                                (mips_lw, spillStruct, stackOffset+spillOffset);
                            afterSpillInst.push_back(load);
                            /*  Increase the spill offset. */
                            spillOffset += 4;
                            /*  return register to pool */
                            registerPool.push_back(spillReg);
                        }
                    }
                }
                /*  After checking the spilled regs it is safe to do the replacement.
                    On these as well. */
                for(SgAsmExpressionPtrList::iterator oIter = opList.begin();
                    oIter != opList.end(); ++oIter) {
                    /* Check that the operand is a register. */
                    if (V_SgAsmDirectRegisterExpression == (*oIter)->variantT()) {
                        /*  Cast to register exporession. */
                        SgAsmDirectRegisterExpression* regPtr = isSgAsmDirectRegisterExpression(*oIter);
                        /*  Decode register and check if it is symbolic. */
                        registerStruct regS = decodeRegister(regPtr);
                        if (symbolic_reg == regS.regName) {
                            /*  Check that it has a register it should have. */
                            if (1 == spillRegs.count(regS.symbolicNumber)) {
                                /*  Get the register, change the regname in the struct. */
                                regS.regName = spillRegs.find(regS.symbolicNumber)->second;
                                /*  Build an register expression. */
                                SgAsmDirectRegisterExpression* regExpr = buildRegister(regS);
                                /*  Set the new expression. */
                                (*oIter) = regExpr;
                            } 
                        }
                    }
                }
                /*  Insert load store instructions ahead of the instruction.. */
                if (0 < beforeSpillInst.size()) {
                    shadowList.insert(shadowList.end(), beforeSpillInst.begin(), beforeSpillInst.end());
                }
                /*  Save the instruction. */
                shadowList.push_back(*stmtIter);
                /*  Insert load store instructions after the instruction. */
                if (0 < afterSpillInst.size()) {
                    shadowList.insert(shadowList.end(), afterSpillInst.begin(), afterSpillInst.end());
                }
                /*  Check if the spillOffset it higher than the max. If so then save it. */
                if (spillOffset > maxSpillOffset) {
                    maxSpillOffset = spillOffset;
                }
            } else {
                /*  The statement is not a mips instruction but we save it. */
                shadowList.push_back(*stmtIter);
            }
        }
        /* Swap the shadowlist and the statementlist. */
        stmtList.swap(shadowList);
    }
}


/* help functions to build load/store instructions */
SgAsmMipsInstruction* linearScanHandler::buildLoadOrStoreSpillInstruction
        (MipsInstructionKind kind, registerStruct destinationOrSource, uint64_t offset) {
    /* Stack pointer register that can be used */
    registerStruct spStruct;
    spStruct.regName = sp; 
    /* a destination register or source register */
//    registerStruct destinationOrSource;
//    destinationOrSource.regName = regname;
    /* Create instruction struct accordingly then build instruction */
    instructionStruct loadstoreStruct;

    if (mips_lw == kind) {
        /*  Settings for the load instructions. */
        loadstoreStruct.kind = mips_lw;
        loadstoreStruct.mnemonic = "lw";
        loadstoreStruct.format = getInstructionFormat(mips_lw);
        /*  Set the destination register. */
        loadstoreStruct.destinationRegisters.push_back(destinationOrSource);
        /*  set the sp in the source register. */
        loadstoreStruct.sourceRegisters.push_back(spStruct);
    } else if (mips_sw == kind) {
        /*  Settings for store instructions. */
        loadstoreStruct.kind = mips_sw;
        loadstoreStruct.mnemonic = "sw";
        loadstoreStruct.format = getInstructionFormat(mips_sw);
        /*  Set the sp to the source register. */
        loadstoreStruct.sourceRegisters.push_back(spStruct);
        /*  set the source register which is being saved. */
        loadstoreStruct.sourceRegisters.push_back(destinationOrSource);
    } else {
        /* Failure, the kind is neither a load or store. */
        ASSERT_not_reachable("Invalid instruction kind passed");
    }
    /*  Set data size and sign bits. */
    loadstoreStruct.memoryReferenceSize = 32;
    loadstoreStruct.significantBits = 32;
    loadstoreStruct.isSignedMemory = true;
    /*  Set the constant of the instruction, which determines the offset. */
    loadstoreStruct.instructionConstant = offset;
    /*  Build the instruction. */
    SgAsmMipsInstruction* mipsInst = buildInstruction(&loadstoreStruct);
    /*  return instruction pointer. */
    return mipsInst;
    
}


/*  Modifies the stack to give space for the linear scans stack needs. */
void linearScanHandler::linearStackModification() {
    /*  Check if the function has activation records by getting the pair
        and checking the pointers. */
    if (debuging) {
        std::cout << "stackOffset: " << std::hex << stackOffset << std::endl;
        std::cout << "maxSpillOffset: " << std::hex << maxSpillOffset << std::endl;
        std::cout << "Combined: " << std::hex << (stackOffset + maxSpillOffset) << std::endl;
    }
    /*  Check if the spillmap is not empty, if so then adjust the stack. */
    if (0 < spillMap.size()) {
        /*  Get the activation set and check if it is empty or not.
            If not empty then go through it and adjust the instructions.
            Otherwise create our own instructions. */
        std::set<SgAsmMipsInstruction*>* activationSet = cfgHandlerPtr->getActivationInstructions();
        if (0 < activationSet->size()) {
            /*  There are activation instructions, modify them. */
            for(std::set<SgAsmMipsInstruction*>::iterator actIter = activationSet->begin();
                actIter != activationSet->end(); ++actIter) {
                /*  Retrieve the operand list and find the constant value. */
                SgAsmExpressionPtrList& operandList = (*actIter)->get_operandList()->get_operands();
                /*  Retrieve the constant of the activation instruction
                    and increase it accordingly. */
                for(SgAsmExpressionPtrList::iterator opIter = operandList.begin();
                    opIter != operandList.end(); ++opIter) {
                    /* Check if it is a constant expression. */
                    if (V_SgAsmIntegerValueExpression == (*opIter)->variantT()) {
                        /*  Cast to correct pointer. */
                        SgAsmIntegerValueExpression* valExpr = isSgAsmIntegerValueExpression(*opIter);
                        /*  Get the constant value. */
                        uint64_t constant = valExpr->get_absoluteValue();
                        /*  Increase the stack space allocated. Means the subtraction value is increased. */
                        constant -= (stackOffset + maxSpillOffset);
                        /*  Set the expression. */
                        valExpr->set_absoluteValue(constant);
                    }
                }
            }
        } else {
            /*  No activation instruction so create instead. */
            instructionStruct myActivation;
            /*  Register struct for the stack pointer. */
            registerStruct spReg;
            spReg.regName = sp;
            /*  Set the kind, mnemonic, format. */
            myActivation.kind = mips_addiu;
            myActivation.mnemonic = "addiu";
            myActivation.format = getInstructionFormat(mips_addiu);
            /*  set the constants for the instruction. */
            myActivation.instructionConstant = -(stackOffset + maxSpillOffset);
            myActivation.significantBits = 32;
            /*  Set the registers to be used in the instruction.
                It is SP as both source and destination. */
            myActivation.sourceRegisters.push_back(spReg);
            myActivation.destinationRegisters.push_back(spReg);
            /*  Build the instruction. */
            SgAsmMipsInstruction* activationMips = buildInstruction(&myActivation);
            /*  Get the entry block pointer. */
            SgAsmBlock* eb = cfgHandlerPtr->getEntryBlock();
            /*  Get the statement list. */
            SgAsmStatementPtrList& entryList = eb->get_statementList();
            /*  Insert the instruction in the beginning of the list. */ 
            entryList.insert(entryList.begin(), activationMips);
        }

        /*  Get the deactivation records and check if there are any instructions
            in the set. */
        std::set<SgAsmMipsInstruction*>* deactivationSet = cfgHandlerPtr->getDeactivationInstructions();
        if (0 < deactivationSet->size()) {
            /* There are deactivation instructions to modify. */
            for(std::set<SgAsmMipsInstruction*>::iterator deIter = deactivationSet->begin();
                deIter != deactivationSet->end(); ++deIter) {
                /*  Get the operand list. */
                SgAsmExpressionPtrList& operandList = (*deIter)->get_operandList()->get_operands();
                /*  Find the constant and modify it. */
                for(SgAsmExpressionPtrList::iterator opIter = operandList.begin();
                    opIter != operandList.end(); ++opIter) {
                    /*  Check if it is the constant. */
                    if (V_SgAsmIntegerValueExpression == (*opIter)->variantT()) {
                        /*  cast to pointer. */
                        SgAsmIntegerValueExpression* valExpr = isSgAsmIntegerValueExpression(*opIter);
                        /*  Get the constant value. */
                        uint64_t constant = valExpr->get_absoluteValue();
                        /*  Adjust the value so all the stack space is returned. */
                        constant += (stackOffset + maxSpillOffset);
                        /*  Set this new value in the instruction. */
                        valExpr->set_absoluteValue(constant);
                    }
                }
            }
        } else {
            /*  Create deactivation instructions. I create one for 
                each exit block. */
            std::set<SgAsmBlock*>* exitBlocks = cfgHandlerPtr->getExitBlocks();
            /*  Iterate through all the exit blocks and add deactivation instructions. */
            for(std::set<SgAsmBlock*>::iterator exitIter = exitBlocks->begin();
                exitIter != exitBlocks->end(); ++exitIter) {
                /*  Create the deactivation record for the block. */
                instructionStruct myDeactivation;
                /*  Register struct for the stack pointer. */
                registerStruct spReg;
                spReg.regName = sp;
                /*  Set the kind, mnemonic, format. */
                myDeactivation.kind = mips_addiu;
                myDeactivation.mnemonic = "addiu";
                myDeactivation.format = getInstructionFormat(mips_addiu);
                /*  set the constants for the instruction. */
                myDeactivation.instructionConstant = stackOffset + maxSpillOffset;
                myDeactivation.significantBits = 32;
                /*  Set the registers to be used in the instruction.
                    It is SP as both source and destination. */
                myDeactivation.sourceRegisters.push_back(spReg);
                myDeactivation.destinationRegisters.push_back(spReg);
                /*  Build the instruction. */
                SgAsmMipsInstruction* deactivationMips = buildInstruction(&myDeactivation);
                /*  Get the statement list. */
                SgAsmStatementPtrList& exitList = (*exitIter)->get_statementList();
                /*  Insert the instruction in as the second to last instruction. */ 
                exitList.insert(--exitList.end(), deactivationMips);
            }
        }
    }
}
