/* Source code for the framework linear scan */



/* includes */
#include "linearScan.hpp"


/*  Constructor */
linearScanHandler::linearScanHandler(CFGhandler* passedCfgObject){
    /*  Set Default values */
    debuging = false;
    /*  reset stack offset. */
    stackOffset = 0;
    /*  Save the pointer to the cfg handler */
    cfgHandlerPtr = passedCfgObject;
    /*  Create the live variable analysis object */
    liveRangeHandler = new liveVariableAnalysisHandler(cfgHandlerPtr->getFunctionCFG());
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
    registerPool.push_front(t8);
    registerPool.push_front(t9);
    /*  extra register to test. */
    //registerPool.push_front(a2);

    //functions in replacement, not any pseudo instructions.
    //registerPool.push_front(v1);

    //does not work in replacement
    //registerPool.push_front(a3);

    //TODO take one register from the pool and put aside as spill register.
    spillReg = registerPool.back();
    registerPool.pop_back();
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
        std::cout << "Replacing symbolic registers and adding spill instructions." << std::endl; 
    }
    replaceSymbolicRegisters();

    /*  After linear scan has been performed modify the stack. If there is no
        stack then create stack instructions. */


}

/*  Replace the registers used by linear scan with symbolic names.
    Each physical register will be replaced with one symbolic register that
    will be used in the whole CFG. This could be considered a greedy method
    since it will result in long live ranges for these registers. */
void linearScanHandler::replaceHardRegisters() {
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
        if (registerPoolSize == activeMap.size()) {
            //SPILL interval.
            if (debuging) {
                std::cout << "Spilling at point: " << intervalIter->first << std::endl;
            }
            spillAtInterval(intervalIter);
        } else {
            /*  Take a register from the pool and remove it form the pool. */
            mipsRegisterName intervalReg = registerPool.front();
            registerPool.pop_front();
            /*  Save the interval and its register. */
            allocationMap.insert(std::pair<unsigned, mipsRegisterName>(intervalIter->second, intervalReg));
            if (debuging) {
                std::cout << "symbolic: " << intervalIter->second << " allocated to register "
                    << getRegisterString(intervalReg) << std::endl;
            }
            mipsRegisterName test = allocationMap.find(intervalIter->second)->second;
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
    if (debuging) {
        std::cout << getRegisterString(spillReg) << " used for spill." << std::endl;
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
        /*  Iterate through the statement list. */
        for(SgAsmStatementPtrList::iterator stmtIter = stmtList.begin();
            stmtIter != stmtList.end(); ++stmtIter) {
            /*  Check if the statement is a mips instruction. */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /*  Cast to mips instruction pointer. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction((*stmtIter));
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

                        /*  If symbolic find the hard register it should have. */

                        /*  Replace the register. */
                    }
                }
            }
        }
    }
}


