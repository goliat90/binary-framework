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
    countSymbolicRegisters();
    /*  Compute def and use for basic blocks */
    computeDefAndUseOnBlocks();
    /*  Compute in and out of basic blocks  */
    computeInOutOnBlocks();
    /*  Compute in and out of each instruction in blocks */
    computeInstructionInOut();
    /*  Determine a traversal order of the blocks and their instructions */
    determineOrderOfDFS();
    /*  Build a live-analysis representation. */
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
            std::cout << "-------------------- block start --------------------" << std::endl;
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
            /*  Print the block def and use. */
            std::cout << "block def: " << currentBits.first;
            std::cout << "block use: " << currentBits.second;
            /* print block limiter */
            std::cout << "-------------------- block end --------------------" << std::endl;
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
                    std::cout << "use: sym_" << (*regiter).symbolicNumber << " ";
                }
            }
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
        been found add them to the defusemap. At the same time the 
        def and use of the current instruction can be set and saved. */
    for(std::set<int>::iterator iter = newUse.begin();
        iter != newUse.end(); ++iter) {
        /* Set the specified bits to true in the use bitset. */
        duPair->second[*iter] = true;
        /*  Set the same bits in the instruction def and use. */
        instructionPair.second[*iter] = true;
    }
    for(std::set<int>::iterator iter = newDef.begin();
        iter != newDef.end(); ++iter) {
        /*  Set the specified bits to true in the define bitset. */
        duPair->first[*iter] = true;
        /*  Set the same bits in the instruction def and use. */
        instructionPair.first[*iter] = true;
    }
    /*  Save the instruction pair to the instruction defuse map */
    defuseInstructionMap.insert(std::pair<SgAsmMipsInstruction*, bitPair>(mipsInst, instructionPair));
    
    /*  debug printout of def and use */
    if(debuging) {
        std::cout << "inst def: " << instructionPair.first << std::endl;
        std::cout << "inst use: " << instructionPair.second << std::endl;
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
                    inoutBits.second = inoutBlockBits.first;
                    /*  Calculate IN */
                    inoutBits.first = defuseBits.second | (inoutBits.second - inoutBits.first);
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
                    inoutBits.first = defuseBits.second | (inoutBits.second - inoutBits.first);
                }
                /*  Save the bits in the inoutInstructionmap */
                inoutInstructionMap.insert(std::pair<SgAsmMipsInstruction*, bitPair>(mips, inoutBits));
            }
        }
    }
    /* All blocks have been visited and the instructions have their IN and OUT calculated */
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
                if (inoutPair.first == oldIN) {
                    modifiedIN = true;
                }
            }
        }
    }
    if (debuging) {
        std::cout << "IN/OUT computation on block level finished." << std::endl;
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


/*  Function performs live-analysis */
void liveVariableAnalysisHandler::computeLiveAnalysis() {

}


/*  Uses the live-variable analysis to find the live intervals  */
void liveVariableAnalysisHandler::findLiveIntervals() {

}

