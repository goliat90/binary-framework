/*  Live-variable analysis.
*   Given a control flow graph the definition and use of variables
*   are computed for each basic block. Then live-variable analysis is applied
*   to the control flow graph, lastly the live intervals are computed.
*/

#include "liveVariableAnalysis.hpp"

/*  Constructor */
liveVariableAnalysisHandler::liveVariableAnalysisHandler(CFG* subroutine) {
    /* Save the cfg */
    functioncfg = subroutine;
}

/*  Set if debuging information should be printed   */
void liveVariableAnalysisHandler::setDebug(bool mode) {
    debuging = mode;
}

/*  Function computes the def and use for each basic block. Iterates through
    each basic block and finds the def and use for the block.   */
void liveVariableAnalysisHandler::computeDefAndUse() {
    /*  First count the number of symbolics and create a mapping between
        symbolics and a bit in the bitsets */
        int bitSetSize = countSymbolicRegisters();

    /*  Iterate through the vertices and all the basic blocks, inspect each
        instruction for def and use of symbolic registers. Save the defs
        and uses for the block. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*functioncfg);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Extract the basic block for the vertex then its statementlist  */
        SgAsmBlock* basic = get(boost::vertex_name, *functioncfg, *iterPair.first);
        SgAsmStatementPtrList& blockStmtList = basic->get_statementList();
        /*  Fresh definition and use set to use for the block   */
        defusePair currentDefUse;
        /*  Bit pair */
        defuseBits currentBit;
        currentBit.first.resize(bitSetSize);
        currentBit.second.resize(bitSetSize);

        /*  Debug code */
        if (debuging) {
            /* print block limiter */
            std::cout << "-------------------- block start --------------------" << std::endl;
        }
        /*  Iterate through the statements and check the registers used */
        for(SgAsmStatementPtrList::iterator stmtIter = blockStmtList.begin();
            stmtIter != blockStmtList.end(); ++stmtIter) {
            /*  Decode the instructions and check the registers for symbolic ones.  */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /* Cast pointer and decode. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                instructionStruct decodedMips = decodeInstruction(mips);
                /* Check the registers in the instruction. */
                instructionUsageAndDefinition(&decodedMips, &currentDefUse);
            }
        }
        /*  Debug code */
        if (debuging) {
            /* print block limiter */
            std::cout << "-------------------- block end --------------------" << std::endl;
        }
        /*  Insert the definition and use pair into the map */
        defuseMap.insert(std::pair<SgAsmBlock*, defusePair>(basic, currentDefUse));
    }
    /* All blocks have had their def and use computed. */
}

/*  Help function to find usage and definition in an instruction   */
void liveVariableAnalysisHandler::instructionUsageAndDefinition(instructionStruct* inst, defusePair* duPair) {
    /*  Temporary sets for defs and use found. */
    std::set<unsigned> newUse;
    std::set<unsigned> newDef;
    /*  Definition of Use: Set of variables whose values may
        be used in Block prior to any definition of the variable. */
    for(std::vector<registerStruct>::iterator regiter = inst->sourceRegisters.begin();
        regiter != inst->sourceRegisters.end(); ++regiter) {
        /* Check if any symbolic registers are used and apply to the def and use rules  */
        if (symbolic_reg == (*regiter).regName) {
            /*  A register is symbolic, should it be added to use or not?
                Check if the register has been defined before. */
            if (1 != duPair->second.count((*regiter).symbolicNumber)) {
                /*  The symbolic register has not been defined yet. So it should
                    be added to the use set. */
                newUse.insert((*regiter).symbolicNumber);
                /*  Debug code */
                if (debuging) {
                    std::cout << "use: sym_" << (*regiter).symbolicNumber << " ";
                }
            }
        }
    }
    /*  Definition of Definition: Set of variables defined in
        block prior to any use of that variable in block. */
    for(std::vector<registerStruct>::iterator regiter = inst->destinationRegisters.begin();
        regiter != inst->destinationRegisters.end(); ++regiter) {
        /* Check if any symbolic registers are used and apply to the def and use rules  */
        if (symbolic_reg == (*regiter).regName) {
            /*  A register is symbolic, should it be added to use or not?
                Check if the register has been used before. */
            if (1 != duPair->first.count((*regiter).symbolicNumber)) {
                /*  The symbolic register has not been used yet. So it should
                    be added to the use set. */
                newDef.insert((*regiter).symbolicNumber);
                /*  Debug code */
                if (debuging) {
                    std::cout << "def: sym_" << (*regiter).symbolicNumber << " ";
                }
            }
        }
    }
    /* new line for debug */
    if (debuging) {
        std::cout << std::endl;
    }
    /*  The registers have been checked for def and use. If any have
        been found add them to the defusemap. */
    if (!newUse.empty()) {
        duPair->first.insert(newUse.begin(), newUse.end());
    }
    if (!newDef.empty()) {
        duPair->second.insert(newDef.begin(), newDef.end());
    }
    //TODO add to bitvector at the same time?
}

/*  Help function to count symbolic registers, also creates a mapping between symbolic
    registers and their corresponding bit in the dynamic bitset. */
int liveVariableAnalysisHandler::countSymbolicRegisters() {
    /* set to track symbolic registers found */
    std::set<unsigned> foundRegs;
    /* Counter for the bit number */
    int bitNum = 0;

    /*  Go through all blocks instructions and registers. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*functioncfg);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Extract the basic block for the vertex then its statementlist  */
        SgAsmBlock* basic = get(boost::vertex_name, *functioncfg, *iterPair.first);
        SgAsmStatementPtrList& blockStmtList = basic->get_statementList();
        /*  Iterate through the statements and check the registers used */
        for(SgAsmStatementPtrList::iterator stmtIter = blockStmtList.begin();
            stmtIter != blockStmtList.end(); ++stmtIter) {
            /*  Decode the instructions and check the registers for symbolic ones.  */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /* Cast pointer and decode. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                instructionStruct decodedMips = decodeInstruction(mips);
                /*  check source registers  */
                for(std::vector<registerStruct>::iterator regiter = decodedMips.sourceRegisters.begin();
                    regiter != decodedMips.sourceRegisters.end(); ++regiter) {
                    /* Check if any symbolic registers are used and save them if the are new */
                    if (symbolic_reg == (*regiter).regName) {
                        if (1 != foundRegs.count((*regiter).symbolicNumber)) {
                            /*  Save the symbolic register  */ 
                            foundRegs.insert((*regiter).symbolicNumber);
                            /*  debuging code */
                            if (debuging) {
                                std::cout << "mapping sym_" << (*regiter).symbolicNumber 
                                << "to bit " << bitNum << std::endl;
                            }
                            /*  Map the symbolic register to a bit  */
                            symbolicToBit.insert(std::make_pair((*regiter).symbolicNumber, bitNum));
                            bitNum++;
                        }
                    }
                }
                /*  Check destination registers */                
                for(std::vector<registerStruct>::iterator regiter = decodedMips.destinationRegisters.begin();
                    regiter != decodedMips.destinationRegisters.end(); ++regiter) {
                    /* Check if any symbolic registers are used and save them if the are new */
                    if (symbolic_reg == (*regiter).regName) {
                        if (1 != foundRegs.count((*regiter).symbolicNumber)) {
                            /*  Save the symbolic register  */ 
                            foundRegs.insert((*regiter).symbolicNumber);
                            /*  debuging code */
                            if (debuging) {
                                std::cout << "mapping sym_" << (*regiter).symbolicNumber 
                                << "to bit " << bitNum << std::endl;
                            }
                            /*  Map the symbolic register to a bit  */
                            symbolicToBit.insert(std::make_pair((*regiter).symbolicNumber, bitNum));
                            bitNum++;
                        }
                    }
                }
            }
        }
    }
    if (debuging) {
        std::cout << "unique symbolics found: " << bitNum << std::endl;
    }
    /* return the number of registers found. */
    return bitNum; 
}

/*  Function performs live-analysis */
void liveVariableAnalysisHandler::computeLiveAnalysis() {

}


/*  Uses the live-variable analysis to find the live intervals  */
void liveVariableAnalysisHandler::findLiveIntervals() {

}

