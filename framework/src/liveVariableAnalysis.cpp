/*  Live-variable analysis.
*   Given a control flow graph the definition and use of variables
*   are computed for each basic block. Then live-variable analysis is applied
*   to the control flow graph, lastly the live intervals are computed.
*/

#include "liveVariableAnalysis.hpp"

/*  Constructor */
liveVariableAnalysisHandler::liveVariableAnalysisHandler(CFG* subroutine) {
    /* Save the cfg */
    functionCFG = subroutine;
    /* Debug printout as default */
    debuging = false;
}

/*  Set if debuging information should be printed   */
void liveVariableAnalysisHandler::setDebug(bool mode) {
    debuging = mode;
}

/*  Performs the live-range analysis, all the steps. */
void liveVariableAnalysisHandler::performLiveRangeAnalysis() {
    /*  First count the number of symbolics and create a mapping between
        symbolics and a bit in the bitsets */
    if (debuging) {
        std::cout << "Counting registers." << std::endl;
    }
    countSymbolicRegisters();

    /*  Compute def and use for basic blocks */
    if (debuging) {
        std::cout << "Computing def and register use." << std::endl;
    }
    computeDefAndUseOnBlocks();

    /*  Compute in and out of basic blocks  */
    if (debuging) {
        std::cout << "Computing in and out on blocks." << std::endl;
    }
    computeInOutOnBlocks();

    /*  Compute in and out of each instruction in blocks */
    if (debuging) {
        std::cout << "Computing in and out on instructions." << std::endl;
    }
    computeInstructionInOut();

    /*  Determine a traversal order of the blocks and their instructions */
    if (debuging) {
        std::cout << "Determining DFS order." << std::endl;
    }
    determineOrderOfDFS();

    /*  Build a live-analysis representation. */
    if (debuging) {
        std::cout << "Building live intervals." << std::endl;
    }
    buildLiveIntervals();    

}

/*  Function computes the def and use for each basic block. Iterates through
    each basic block and finds the def and use for the block.   */
void liveVariableAnalysisHandler::computeDefAndUseOnBlocks() {
    /*  Iterate through the vertices and all the basic blocks, inspect each
        instruction for def and use of symbolic registers. Save the defs
        and uses for the block. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*functionCFG);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Extract the basic block for the vertex then its statementlist  */
        SgAsmBlock* basic = get(boost::vertex_name, *functionCFG, *iterPair.first);
        SgAsmStatementPtrList& blockStmtList = basic->get_statementList();
        /*  Bit pair */
        boost::dynamic_bitset<> first (numberOfVariables);
        boost::dynamic_bitset<> second (numberOfVariables);
        bitPair currentBits (first, second);
        /*  Debug code */
        if (debuging) {
            /* print block limiter */
            std::cout << "-------------------- block:" << std::hex << basic->get_id() << " start --------------------" << std::endl;
        }
        /*  Iterate through the statements and check the registers used */
        for(SgAsmStatementPtrList::iterator stmtIter = blockStmtList.begin();
            stmtIter != blockStmtList.end(); ++stmtIter) {
            /*  Decode the instructions and check the registers for symbolic ones.  */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /* Check the registers in the instruction. */
                instructionUsageAndDefinition((*stmtIter), &currentBits);
            }
        }
        /*  Debug code */
        if (debuging) {
            std::cout << std::endl;
            /*  Print the block def and use. */
            std::cout << "block def: "; //<< currentBits.first; // << std::endl;
            for(int i = 0; i < currentBits.first.size(); i++) {
                if (true == currentBits.first[i]) {
                    std::cout << std::dec << i << " ";
                }
            }
            std::cout << std::endl;
            std::cout << "block use: "; // << currentBits.second << std::endl;
            for(int i = 0; i < currentBits.second.size(); i++) {
                if (true == currentBits.second[i]) {
                    std::cout << std::dec << i << " ";
                }
            }
            std::cout << std::endl;
            /* print block limiter */
            std::cout << "-------------------- block:" << std::hex << basic->get_id() << " end --------------------" << std::endl;
        }
        /*  Insert the definition and use pair into the map */
        defuseBlockMap.insert(std::pair<SgAsmBlock*, bitPair>(basic, currentBits));
    }
    /* All blocks have had their def and use computed. */
}

/*  Help function to find usage and definition in an instruction, which is
    added to the def use bits of the block. At the same time the def and use
    of for the instruction is set. */
void liveVariableAnalysisHandler::instructionUsageAndDefinition(SgAsmStatement* asmInst, bitPair* duPair) {
    /* Cast pointer and decode. */
    SgAsmMipsInstruction* mipsInst = isSgAsmMipsInstruction(asmInst);
    instructionStruct inst = decodeInstruction(mipsInst);
    /*  Temporary sets for defs and use found in the inspected instruction. */
    std::set<int> newUse;
    std::set<int> newDef;
    /*  Bitset pair for the instruction being inspected. */
    boost::dynamic_bitset<> first (numberOfVariables);
    boost::dynamic_bitset<> second (numberOfVariables);
    bitPair instructionPair (first, second);
    /*  Definition of Use: Set of variables whose values may
        be used in Block prior to any definition of the variable. */
    for(std::vector<registerStruct>::iterator regiter = inst.sourceRegisters.begin();
        regiter != inst.sourceRegisters.end(); ++regiter) {
        /* Check if any symbolic registers are used and apply to the def and use rules  */
        if (symbolic_reg == (*regiter).regName) {
            /*  A register is symbolic, should it be added to use or not?
                Check if the register has been defined before. */
            int symBit = symbolicToBit.find((*regiter).symbolicNumber)->second;
            /*  Check if the bit is set in the defined bitset */
            if (!duPair->first[symBit]) {
                /*  The symbolic register has not been defined yet. So it should
                    be added to the use set. */
                newUse.insert(symBit);
                /*  Debug code */
                if (debuging) {
                    std::cout << std::endl << "block identified use: sym_" << std::dec << (*regiter).symbolicNumber << " ";
                }
            }
            //TODO Need to add code here that sets the bit for the def use pair for the instruction
            /*  Set the bit for the use bit in the instruction def/use. */
            instructionPair.second[symBit] = true;
        }
    }
    /*  Definition of Definition: Set of variables defined in
        block prior to any use of that variable in block. */
    for(std::vector<registerStruct>::iterator regiter = inst.destinationRegisters.begin();
        regiter != inst.destinationRegisters.end(); ++regiter) {
        /* Check if any symbolic registers are used and apply to the def and use rules  */
        if (symbolic_reg == (*regiter).regName) {
            /*  A register is symbolic, should it be added to use or not?
                Check if the register has been used before. */
            int symBit = symbolicToBit.find((*regiter).symbolicNumber)->second;
            /*  Check if the bit is set in the use bitset */
            if (!duPair->second[symBit]) {
                /*  The symbolic register has not been used yet. So it should
                    be set to true in the defined set. */
                newDef.insert(symBit);
                /*  Debug code */
                if (debuging) {
                    std::cout << "block identiefied def: sym_" << std::dec << (*regiter).symbolicNumber << " ";
                }
            }
            //TODO Need to add code here that sets the bit for the def use pair for the instruction
            /*  Set the bit for the def bit in the instruction def/use. */
            instructionPair.first[symBit] = true;
        }
    }
    /* new line for debug */
    if (debuging) {
        std::cout << std::endl;
    }
    /*  The registers have been checked for def and use. If any have
        been found add them to the defusemap. At the same time the 
        def and use of the current instruction can be set and saved. */
    for(std::set<int>::iterator iter = newUse.begin();
        iter != newUse.end(); ++iter) {
        /* Set the specified bits to true in the use bitset. */
        duPair->second[*iter] = true;
        /*  Set the same bits in the instruction def and use. */
        //instructionPair.second[*iter] = true;
    }
    for(std::set<int>::iterator iter = newDef.begin();
        iter != newDef.end(); ++iter) {
        /*  Set the specified bits to true in the define bitset. */
        duPair->first[*iter] = true;
        /*  Set the same bits in the instruction def and use. */
        //instructionPair.first[*iter] = true;
    }
    /*  Save the instruction pair to the instruction defuse map */
    defuseInstructionMap.insert(std::pair<SgAsmMipsInstruction*, bitPair>(mipsInst, instructionPair));
    
    /*  debug printout of def and use */
    if(debuging) {
        std::cout << std::hex << mipsInst->get_address() << " inst " << mipsInst->get_mnemonic() << " def: "; //<< instructionPair.first << std::endl;
        for(int i = 0; i < instructionPair.first.size(); i++) {
            if (true == instructionPair.first[i]) {
                std::cout << std::dec << i << " ";// std::end;
            }
        }
        std::cout << std::endl;
        std::cout << std::hex << mipsInst->get_address() << " inst " << mipsInst->get_mnemonic() << " use: "; // << instructionPair.second << std::endl;
        for(int i = 0; i < instructionPair.second.size(); i++) {
            if (true == instructionPair.second[i]) {
                std::cout << std::dec << i << " ";// std::end;
            }
        }
    }
}

/*  Help function to count symbolic registers, also creates a mapping between symbolic
    registers and their corresponding bit in the dynamic bitset. */
void liveVariableAnalysisHandler::countSymbolicRegisters() {
    /* set to track symbolic registers found */
    std::set<unsigned> foundRegs;
    /* Counter for the bit number */
    int bitNum = 0;

    /*  Go through all blocks instructions and registers. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*functionCFG);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Extract the basic block for the vertex then its statementlist  */
        SgAsmBlock* basic = get(boost::vertex_name, *functionCFG, *iterPair.first);
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
                            //if (debuging) {
                            //    std::cout << "mapping sym_" << (*regiter).symbolicNumber 
                            //    << " to bit " << bitNum << std::endl;
                            //}
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
                            //if (debuging) {
                            //    std::cout << "mapping sym_" << (*regiter).symbolicNumber 
                            //    << " to bit " << bitNum << std::endl;
                            //}
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
        /*  Printout the mapping */
        for(std::map<unsigned, int>::iterator iter = symbolicToBit.begin();
            iter != symbolicToBit.end(); ++iter) {
            std::cout << "symbolic: " << iter->first << " mapped to bit: " << iter->second << std::endl;
        }
    }
    /*  Save the number of variables found */
    numberOfVariables = bitNum;
}

/*  Compute IN and OUT on individual instructions, computed by going through
    instructions in a block, from last to first. */
void liveVariableAnalysisHandler::computeInstructionInOut() {
    /*  Go through all the vertices, extract their block, get the statementlist */
    for(std::pair<CFGVIter, CFGVIter> vertPair = vertices(*functionCFG);
            vertPair.first != vertPair.second; ++vertPair.first) {
        /*  Boolean to track if the first instruction has been calculated */
        bool firstInstruction = true;
        /*  Get the basic block from the vertex */
        SgAsmBlock* basic = get(boost::vertex_name, *functionCFG, *vertPair.first);
        /*  If debuging print out block address */
        if (debuging) {
            std::cout << "Calculating IN/OUT on instructions in block: " << basic->get_id() << std::endl;
        }
        /*  extract the statementlist so it can be iterated */
        SgAsmStatementPtrList& stmtList = basic->get_statementList();
        /*  Iterate the backward through each blocks instruction */
        for(SgAsmStatementPtrList::reverse_iterator stmtIter = stmtList.rbegin();
            stmtIter != stmtList.rend(); ++stmtIter) {
            /* calculate IN and OUT for each instruction */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /*  Cast the to mips instruction. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                /*  Get the def use bits for the instruction. */
                bitPair defuseBits = defuseInstructionMap.find(mips)->second;
                /*  Set up the inout bits */
                boost::dynamic_bitset<> first(numberOfVariables);
                boost::dynamic_bitset<> second(numberOfVariables);
                bitPair inoutBits (first, second);

                if (firstInstruction) {
                    //Special case, the first(last) instruction calculated uses
                    //the OUT is calculated with the IN of the block
                    /*  Get the IN and OUT for the block */
                    bitPair inoutBlockBits = inoutBlockMap.find(basic)->second;
                    /*  Calculate OUT, is done by using the OUT bitset from
                        the current block as IN. It is basically just copying it.  */
                    //TODO inoutBlockBits was .first, which means it used IN, 
                    //TODO it is set to .second now. which should be OUT.
                    inoutBits.second = inoutBlockBits.second;
                    /*  debug printing. */
                    if (debuging) {
                        std::cout << " Using block OUT: ";
                        for(int i = 0; i < inoutBits.second.size(); i++) {
                            if (true == inoutBits.second[i]) {
                                std::cout << std::dec << i << " ";// std::end;
                            }
                        }
                    }
                    /*  Calculate IN */
                    inoutBits.first = defuseBits.second | (inoutBits.second - defuseBits.first);
                    /*  Set the bool to false since the special case has been handled */
                    firstInstruction = false;
                } else {
                    /*  Other instructions in the block. */
                    /*  Retrieve the previous instruction pointer, i need previous IN */
                    SgAsmMipsInstruction* prevMips = isSgAsmMipsInstruction(*(stmtIter - 1));
                    /*  Calculate OUT, requires the IN of the previous instruction. */
                    boost::dynamic_bitset<> prevIN = inoutInstructionMap.find(prevMips)->second.first;
                    inoutBits.second = prevIN;
                    /*  Calculate IN */
                    inoutBits.first = defuseBits.second | (inoutBits.second - defuseBits.first);
                }
                /*  Print the IN/OUT set, if debuging is active. */
                if (debuging) {
                    std::cout << std::hex << mips->get_address() << " inst  IN: "; // << inoutBits.first << std::endl;
                    for(int i = 0; i < inoutBits.first.size(); i++) {
                        if (true == inoutBits.first[i]) {
                            std::cout << std::dec << i << " ";// std::end;
                        }
                    }
                    std::cout << std::endl;
                    std::cout << std::hex << mips->get_address() << " inst OUT: "; // << inoutBits.second << std::endl;
                    for(int i = 0; i < inoutBits.second.size(); i++) {
                        if (true == inoutBits.second[i]) {
                            std::cout << std::dec << i << " ";// std::end;
                        }
                    }
                    std::cout << std::endl;
                }
                /*  Save the bits in the inoutInstructionmap */
                inoutInstructionMap.insert(std::pair<SgAsmMipsInstruction*, bitPair>(mips, inoutBits));
            }
        }
        /*  If debuging print out block address */
        if (debuging) {
            std::cout << "IN/OUT calculations finished on instructions in block: " << std::hex << basic->get_id() << std::endl;
        }
    }
    /* All blocks have been visited and the instructions have their IN and OUT calculated */
    /*  If debuging print the IN/OUT map values. */
    if (debuging) {
        
    }
}

/*  Compute In and Out on basic blocks. */
/* Using the algorithm from the Dragons book on live variable analysis. */
void liveVariableAnalysisHandler::computeInOutOnBlocks() {
    /*  Add ENTRY and EXIT vertices to the cfg */
    addEntryExit();
    /*  Initialize the in and out bitsets, set to zero */
    initializeInOutOnBlocks();
    
    bool modifiedIN = true;
    /*  Continue as long as any changes in an IN occurs. */
    while (modifiedIN) {
        /*  Set modifiedIN to false, if an IN gets updated it is set to true */
        modifiedIN = false;
        /*  For each block except EXIT. Compute OUT then IN. */
        for(std::map<SgAsmBlock*, bitPair>::iterator iter = inoutBlockMap.begin();
            iter != inoutBlockMap.end(); ++iter) {
            /* Check so we do not compute on the exit block */
            if ((*iter).first != blockEXIT) {
                /*  Retrieve the block inout pair */
                bitPair inoutPair = (*iter).second;
                /*  Save the the old IN to check if it has been updated later */
                boost::dynamic_bitset<> oldIN = inoutPair.first;
                /*  Retrieve the def and use pair of the block */
                bitPair defusePair = defuseBlockMap.find((*iter).first)->second;
                /*  OUT computation, union of all the successor blocks IN. */
                inoutPair.second = computeOutOnBlock((*iter).first);
                /*  IN computation, block use unioned (OUT from block - block def) */
                inoutPair.first =  defusePair.second | (inoutPair.second - defusePair.first);
                /*  Compare latest computed IN with previous IN. If they differ
                    then set modifiedIN to true */
                if (inoutPair.first != oldIN) {
//                    if (debuging) {
//                        std::cout << "IN changed Block:"  << std::hex << (*iter).first->get_id() << std::endl;
//                        std::cout << "OLD: " << oldIN << std::endl;
//                        std::cout << "NEW: " << inoutPair.first << std::endl;
//                        std::cout << "OUT: " << inoutPair.second << std::endl;
//                    }
                    modifiedIN = true;
                }
                /*  Save the new IN and OUT computation */
                (*iter).second = inoutPair;
            }
        }
    }
    if (debuging) {
        std::cout << "IN/OUT computation on block level finished." << std::endl;
        for(std::map<SgAsmBlock*, bitPair>::iterator iter = inoutBlockMap.begin();
            iter != inoutBlockMap.end(); ++iter) {
            /*  Get the bit pair. */
            bitPair inoutPair = (*iter).second;
            /*  print out the in out bits that are set. */
            std::cout << "Block "  << std::hex << (*iter).first->get_id() << "  IN: ";
            for(int i = 0; i < inoutPair.first.size(); i++) {
                if (true == inoutPair.first[i]) {
                    std::cout << std::dec << i << " ";
                }
            }
            std::cout << std::endl;

            std::cout << "Block "  << std::hex << (*iter).first->get_id() << " OUT: ";
            for(int i = 0; i < inoutPair.second.size(); i++) {
                if (true == inoutPair.second[i]) {
                    std::cout << std::dec << i << " ";
                }
            }
            std::cout << std::endl;
        }
    }
    /*  Remove ENTRY and EXIT from the cfg */
    removeEntryExit();
}


/*  Function to compute OUT for the live-range analysis on blocks */
boost::dynamic_bitset<> liveVariableAnalysisHandler::computeOutOnBlock(SgAsmBlock* rootBlock) {
    /*  Variables */
    CFG::vertex_descriptor rootVertex;
    /*  Temporary bitset to calculate the OUT into */
    boost::dynamic_bitset<> outBits(numberOfVariables);

    /*  Find the vertex that we want the successors of, search all vertices
        and check the sgasmblock pointer. Save it, then revtireve the successors. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*functionCFG);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /* Get the rootBlock. */
        SgAsmBlock* currentBlock = get(boost::vertex_name, *functionCFG, *iterPair.first);
        /*  Compare the pointers */
        if (currentBlock == rootBlock) {
            /* Save the vertex */
            rootVertex = *iterPair.first;
            break;
        }
    }
    /*  Iterate over all the successors and perform the OR operations. */
    for(std::pair<CFGOEIter, CFGOEIter> edgePair = out_edges(rootVertex, *functionCFG);
        edgePair.first != edgePair.second; ++edgePair.first) {
        /*  Get the vertex descriptor for the target vertice, i.e a successor vertice */
        CFG::vertex_descriptor successorVertex = target(*edgePair.first, *functionCFG);
        /*  Retrieve the block pointer and with it the in IN bitset */
        SgAsmBlock* edgeBlock = get(boost::vertex_name, *functionCFG, successorVertex);
        /*  Get the successor bitset */
        bitPair successorPair = inoutBlockMap.find(edgeBlock)->second;
        /*  perform union operation and compute the OUT bits */
        outBits = outBits | successorPair.first;
    }
    /*  Return the out bits. */
    return outBits;
}


/*  Initializes the parameters before computations of IN and OUT on blocks */
void liveVariableAnalysisHandler::initializeInOutOnBlocks() {
    /*  Go through all vertices and their block inialize the bitsets */
    for(std::pair<CFGVIter, CFGVIter> itPair = vertices(*functionCFG);
        itPair.first != itPair.second; ++itPair.first) {
        /*  Initialize the in out bitset */
        boost::dynamic_bitset<> first (numberOfVariables);
        boost::dynamic_bitset<> second (numberOfVariables);
        bitPair inoutPair (first, second);
        /*  Get the basic block pointer and add it to the map. */
        SgAsmBlock* block = get(boost::vertex_name, *functionCFG, *itPair.first);
        inoutBlockMap.insert(std::pair<SgAsmBlock*, bitPair>(block, inoutPair));
    } 
}

/*  add ENTRY and EXIT vertices to the cfg */
void liveVariableAnalysisHandler::addEntryExit() {
    /* find the first and last block in the cfg. */
    std::set<CFG::vertex_descriptor> targetVertices;
    std::set<CFG::vertex_descriptor> sourceVertices;

    /* retrieve each edges target and source and save */
    for(std::pair<CFGEIter, CFGEIter> edgeIter = edges(*functionCFG);
        edgeIter.first != edgeIter.second; ++edgeIter.first) {
        targetVertices.insert(target(*edgeIter.first, *functionCFG));
        sourceVertices.insert(source(*edgeIter.first, *functionCFG));
    }

    /* At this point all vertices that are either a target or a source
        for a edge is saved. The first and last block will only be
        present as a target or source. */
    for(std::set<CFG::vertex_descriptor>::iterator iter = sourceVertices.begin();
        iter != sourceVertices.end(); ++iter) {
        /* Find the source vertex that is not among the target vertices.
            That vertex is the entry vertex */
        if (targetVertices.count(*iter) == 0) {
            /*  The current vertex is the first block in the cfg,
                add ENTRY vertex and connect it to this block. */
            ENTRY = add_vertex(*functionCFG);
            /*  Create an empty SgAsmBlock to be used as key, save it. */
            blockENTRY = new SgAsmBlock();
            /*  Add the block to the property map. */
            put(boost::vertex_name, *functionCFG, ENTRY, blockENTRY);
            /*  Add an edge between ENTRY and the first block */
            add_edge(ENTRY, (*iter), *functionCFG);
            /*  Additionally, save the block pointer to the entry block
                of the CFG. I need it later in DFS traversal */
            rootVertex = (*iter);
            cfgRootBlock = get(boost::vertex_name, *functionCFG, (*iter));
        }
    }
    for(std::set<CFG::vertex_descriptor>::iterator iter = targetVertices.begin();
        iter != targetVertices.end(); ++iter) {
        /* Find the target vertex that is not among the source vertices.
            That vertex is the exit vertex */
        if (sourceVertices.count(*iter) == 0) {
            /*  The current vertex is the last block in the cfg,
                add EXIT vertex and connect it to this block. */
            EXIT = add_vertex(*functionCFG);
            /*  Create an empty SgAsmBlock to be used */
            blockEXIT = new SgAsmBlock();
            /*  Add the block to the property map. */
            put(boost::vertex_name, *functionCFG, EXIT, blockEXIT);
            /*  Add an edge between ENTRY and the first block */
            add_edge(EXIT, (*iter), *functionCFG);
        }
    }
    /*  I need def and use on the ENTRY block, adding it here to the defuseblockmap. */
    boost::dynamic_bitset<> first(numberOfVariables);
    boost::dynamic_bitset<> second(numberOfVariables);
    bitPair entryPair (first, second);
    defuseBlockMap.insert(std::pair<SgAsmBlock*, bitPair>(blockENTRY, entryPair));
}


/*  remove ENTRY and EXIT vertices, restoring the cfg. */
void liveVariableAnalysisHandler::removeEntryExit() {
    /* Clear the edges from the vertices and then remove the vertices */
    clear_vertex(ENTRY, *functionCFG);
    clear_vertex(EXIT, *functionCFG);
    remove_vertex(ENTRY, *functionCFG);
    remove_vertex(EXIT, *functionCFG);
    /*  Remove the defuse block entry for the ENTRY block */
    defuseBlockMap.erase(blockENTRY);
}


/*  Function that determines a DFS order that will be used in the live-range
    analysis. */
void liveVariableAnalysisHandler::determineOrderOfDFS() {
    /*  Create a list that will contain block pointers in the orders visited */
    std::list<SgAsmBlock*> blockOrder;
    /*  Create a dfs visitor object */
    liveDFSVisitor dfsObject;
    /*  Pass the reference to the object */
    dfsObject.passListReference(&blockOrder);
    /*  Call of the depth-first search function, second and third argument is through
        boost bgl named parameters. */
    depth_first_search(*functionCFG,
        boost::visitor(dfsObject).
        root_vertex(rootVertex));
    /*  If debuging is set then print the block order. */
    if (debuging) {
        std::cout << "Block order determined by DFS." << std::endl;
        /*  Print the list with block pointers */
        for(std::list<SgAsmBlock*>::iterator bpIter = blockOrder.begin();
            bpIter != blockOrder.end(); ++bpIter) {
            /*  Print the block address */
            std::cout << "Block: " << std::hex << (*bpIter)->get_id() << std::endl;
        }
    }

    /*  The blockOrder list should now be in a order that is acceptable
        Go through it and create a list of instructions. Consisting of the
        order the blocks have been sorted. */
    int DFSnumber = 0;
    for(std::list<SgAsmBlock*>::iterator iter = blockOrder.begin();
        iter != blockOrder.end(); ++iter) {
        /*  Get the statement list and copy it to the list. */
        SgAsmStatementPtrList& blockList = (*iter)->get_statementList();
        /*  Iterate through the statement list and cast to mips instructions.
            I do this since all information related to instructions are mapped
            to Mips instruction pointers. */
        for(SgAsmStatementPtrList::iterator  stmtIter = blockList.begin();
            stmtIter != blockList.end(); ++stmtIter) {
            /*  Verify that it is a mips instruction */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /*  Cast to mips instruction pointer. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                /*  Save the pointer, push to back. */
                DFSInstructionOrder.push_back(std::pair<int, SgAsmMipsInstruction*>(DFSnumber, mips));
                /* Increment number counter */
                DFSnumber++;
            }
        }
    }
    /* Print the instruction list if debuging is set */
    if (debuging) {
        std::cout << "DFS Instruction order." << std::endl;
        for(std::list<std::pair<int, SgAsmMipsInstruction*> >::iterator iter = DFSInstructionOrder.begin();
            iter != DFSInstructionOrder.end(); ++iter) {
            /*  Decode the instruction */
            instructionStruct inst = decodeInstruction((*iter).second);
            /*  Print the dfs order number. */
            std::cout << "DFS num " << std::dec << (*iter).first << "  -- ";
            /*  Print instructions */
            printInstruction(&inst);
        }
        std::cout << "-------------------" << std::endl;
    }
}


/*  Uses the live-variable analysis to build a live interval representation. */
void liveVariableAnalysisHandler::buildLiveIntervals() {

    /*  Find for each symbolic where it starts in the dfs order. */
    for(std::map<unsigned, int>::iterator symbolIter = symbolicToBit.begin();
        symbolIter != symbolicToBit.end(); ++symbolIter) {
        /*  By iterating through the dfs order from the beginning i will find the
            beginning of an interval. */
        /*  Get the bit index for the symbolic. */
        int bitNum = symbolIter->second;
        for(std::list<std::pair<int, SgAsmMipsInstruction*> >::iterator dfsIter = DFSInstructionOrder.begin();
            dfsIter != DFSInstructionOrder.end(); ++dfsIter) {
            /*  For each instruction check the IN/OUT bit pair for the specific
                symbolic which is mapped to a specific bit. */
            bitPair instInOut = inoutInstructionMap.find((*dfsIter).second)->second;
            /*  Check the bit for the symbolic being checked. If it is set then the startpoint
                has been found. Check if OUT is set for the specific bit in the instruction.
                Then it becomes live in this instruction. */
            if (instInOut.second[bitNum]) {
                /*  Save the where the instruction is in the dfs order (number)
                    and the symbolic number related to the range. */
                if (debuging) {
                    std::cout << "Start interval found at DFS number: " << dfsIter->first
                        << " for symbolic " << symbolIter->first << std::endl;
                }
                startPointBiMap.insert(intervalMap::value_type(dfsIter->first, symbolIter->first));
                /*  When we find an interval we also add an end point which is the last
                    in the dfs order. */
                //TODO test 
                //endPointBiMap.insert(intervalMap::value_type((DFSInstructionOrder.size() - 1), symbolIter->first));
                /*  When the interval has been found we can break from the loop */
                break;
            }
        }

        /*  Iterate backwards and find where the live range ends for the current symolic. */
        for(std::list<std::pair<int, SgAsmMipsInstruction*> >::reverse_iterator dfsIterRev = DFSInstructionOrder.rbegin();
            dfsIterRev != DFSInstructionOrder.rend(); ++dfsIterRev) {
            /*  For each instruction check the IN/OUT bit pair for the specific
                symbolic which is mapped to a specific bit. */
            bitPair instInOut = inoutInstructionMap.find((*dfsIterRev).second)->second;
            /*  Check if the bit for the symbolic is set on IN but not OUT. This
                indicates that the symbolic has been used and the range ends here. */
            if (instInOut.first[bitNum] && !instInOut.second[bitNum]) {
                /*  Save the live interval end point and the number of the symbolic */
                if (debuging) {
                    std::cout << "End of interval found at DFS number: " << dfsIterRev->first
                        << " for symbolic " << symbolIter->first << std::endl;
                }
                //TODO do replace here instead.
                endPointBiMap.insert(intervalMap::value_type(dfsIterRev->first, symbolIter->first));
                /*  Break from the loop. */
                break;
            }
        }
    }

    /*  Print out the live intervals if debug is set. */
    if (debuging) {
        /*  Using the start point map and then with the symbolic
            retrieve the endpoint from endpoint map. */
        std::cout << "Start Points." << std::endl;
        for(intervalMap::iterator startIter = startPointBiMap.begin();
            startIter != startPointBiMap.end(); ++startIter) {
            /*  left = interval point, right = symbol */
            std::cout << "Symbolic: " << startIter->right << " Start: " << startIter->left << std::endl;
        }

        std::cout << std::endl << " End points." << std::endl;
        for(intervalMap::iterator endIter = endPointBiMap.begin();
            endIter != endPointBiMap.end(); ++endIter) {
            /*  left = interval point, right = symbol */
            std::cout << "Symbolic: " << endIter->right << " End: " << endIter->left << std::endl;
        }
        std::cout << std::endl;
    }
}


/*  Custom discover_vertex function for the dfs search. It will set save the
    block pointers extracted from the vertices */
void liveDFSVisitor::discover_vertex(CFGVertex newVertex, const CFG& g) const {
    /* From the vertex extract the SgAsmBlock pointer and save it. */
    SgAsmBlock* vertexBlock = get(boost::vertex_name, g, newVertex);
    /* Insert the block pointer at the end of the list */
    dfsBlockOrder->push_back(vertexBlock);
}


/*  Function that sets a reference to a list that store the visit order */
void liveDFSVisitor::passListReference(std::list<SgAsmBlock*>* reference) {
    /*  Save the pointer */
    dfsBlockOrder = reference;
}
