/* CFG Handler */

#include "cfgHandler.hpp"

/* setup for this class */
void CFGhandler::initialize(SgProject* root) {
    /*  Debugging bool set to false as default. */
    debugging = false;
    /* With the SgProject build the programcfg and save it */
    std::vector<SgAsmInterpretation*> interpretation = SageInterface::querySubTree<SgAsmInterpretation>(root);
    /* build cfg. */
    rose::BinaryAnalysis::ControlFlow cfganalyzer;
    programCFG = new CFG;
    cfganalyzer.build_block_cfg_from_ast(interpretation.back(), *programCFG);
}

/* returns the function cfg */
CFG* CFGhandler::getFunctionCFG() {
    return functionCFG;
}

/* returns the program cfg */
CFG* CFGhandler::getProgramCFG() {
    return programCFG;
}

/* Get the new address for the instruction */
rose_addr_t CFGhandler::getNewAddress(rose_addr_t oldAddress) {
    /* search address map for entry */
    return instructionMap.find(oldAddress)->second;
}


/* Check if the address has been relocated */
bool CFGhandler::hasNewAddress(rose_addr_t instructionAddress) {
    /* search address map for entry */
    int found = instructionMap.count(instructionAddress);
    /* address i present then found is 1, otherwise 0 */
    if (found == 1) {
        return true;
    } else {
        return false;
    }
}


/* Is the instruction allowed to be transformed? */
bool CFGhandler::isForbiddenInstruction(SgAsmMipsInstruction* inst) {
    /*  See if the instruction is present in the set. */
    if (1 == forbiddenInstruction.count(inst)) {
        return true;
    } else {
        return false;
    }
}

/* Determines the entry and exit block of the functions cfg. */
void CFGhandler::findEntryAndExitBlocks() {
    /*  To find the entry block i can get it returned by getting
        the enclosing function from a basicblock. */
    std::pair<CFGVIter, CFGVIter> verticePair = vertices(*functionCFG);
    SgAsmBlock* block = get(boost::vertex_name, *functionCFG, *verticePair.first);
    SgAsmFunction* function = block->get_enclosing_function();
    entryBlock = function->get_entry_block();
    /*  Debug print. */
    if (debugging) {
        std::cout << "Block " << std::hex << entryBlock->get_address()
            << " is entry block." << std::endl;
    }
    //TODO if i get all vertices here and just get the block
    //TODO from that i can get the enclosing function and the entry block.

    //TODO I could use that vertice set and iterate over it
    //TODO and identify the exit block(s). the exit block(s)
    //TODO can be identified by having only inbound edges but no outbound.

    /*  Check how many vertices there actually are in the cfg
        to determine the way we determine exit blocks.
        If there are only two, which means one basic block and
        the delay block it is handled specially. */
    //TODO I need to take consideration if i only have one basicblock and
    //TODO delay block i should skip this and set the entry block as exit block as well.
    if (2 >= num_vertices(*functionCFG)) {
        /*  There is only one basic block and a delay slot block
            or even less. Save the entry block as an exit block. */
        exitBlocks.insert(entryBlock);
        /*  Debug print. */
        if (debugging) {
            std::cout << "Block " << std::hex << entryBlock->get_address()
                << " is also exit block." << std::endl;
        }
    } else {
        /*  There are more than two vertices. This means there can
            possibly be one or more blocks that can be considered exit blocks. */
        for(verticePair; verticePair.first != verticePair.second;
            ++verticePair.first) {
            /*  Check the vertices if they classify as an exit block.
                Have no out edges and have at least one in-edges,
                the at least one is to avoid delay slot vertices. */
            int inDeg = in_degree(*verticePair.first, *functionCFG);
            int outDeg = out_degree(*verticePair.first, *functionCFG);
            if (0 < inDeg && 0 == outDeg) {
                /*  The vertice fits the exit block requirements, save it. */
                SgAsmBlock* exitBlock = get(boost::vertex_name, *functionCFG, *verticePair.first);
                /*  Debug print. */
                if (debugging) {
                    std::cout << "Block " << std::hex << exitBlock->get_address()
                        << " considered an exit block." << std::endl;
                }
                /*  Save the block pointer. */
                exitBlocks.insert(exitBlock);
            }
        }
    }
}

/*  Identifies the activation record instruction. It achieves it
    by traversing the entry block and the exit blocks. */
void CFGhandler::findActivationRecords() {
    /*  First iterate through the statementlist of the
        exit block. */
    SgAsmStatementPtrList& entryList = entryBlock->get_statementList();
    /*  Iterate through the statements and look for instructions
        that can be classified as activation record instructions. */
    for(SgAsmStatementPtrList::iterator listIter = entryList.begin();
        listIter != entryList.end(); ++listIter) {
        /*  Check if the pointer is an instruction. */
        if (V_SgAsmMipsInstruction == (*listIter)->variantT()) {
            /*  Cast the pointer to an mips instruction. */
            SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*listIter);
            /*  Check if the instruction is an activation record instruction.
                If it is an addiu then we check the registers used. */
            if (mips_addiu == mips->get_kind()) {
                /*  The instruction is the right type, check if the registers
                    used is the stack pointer, sp. */
                bool isActivation = true;
                /*  Get the operand list and check them. */
                SgAsmExpressionPtrList& opExprList = mips->get_operandList()->get_operands();
                /*  Check the operands. */
                for(SgAsmExpressionPtrList::iterator opIter = opExprList.begin();
                    opIter != opExprList.end(); ++opIter) {
                    /*  Check if the expression is a registerexpression. */
                    if (V_SgAsmDirectRegisterExpression == (*opIter)->variantT()) {
                        /*  Decode the register and check it. If it is not SP then break. */
                        registerStruct regS = decodeRegister(*opIter);
                        if (sp != regS.regName) {
                            /*  The register is not sp. It is not a forbidden instruction. */
                            isActivation = false;
                            break;
                        }
                    }
                }
                /*  The instruction is an activation so it needs to be stored
                    and set as a forbidden instruction. */
                if (true == isActivation) {
                    activationInstruction.insert(mips);
                    forbiddenInstruction.insert(mips);
                    if (debugging) {
                        std::cout << "forbidden activation instruction found: " << std::hex << mips->get_address() << std::endl;
                    }
                }
            }
        }
    }

    /*  Search through the exit block(s) for deactivation instructions. */
    for(std::set<SgAsmBlock*>::iterator exitIter = exitBlocks.begin();
        exitIter != exitBlocks.end(); ++exitIter) {
        /*  Get the statement list. */
        SgAsmStatementPtrList& exitList = (*exitIter)->get_statementList();
        /*  Go through the statement list looking for forbidden instructions. */
        for(SgAsmStatementPtrList::iterator listIter = exitList.begin();
            listIter != exitList.end(); ++listIter) {
            /*  Check if the pointer is an instruction. */
            if (V_SgAsmMipsInstruction == (*listIter)->variantT()) {
                /*  Cast the pointer to an mips instruction. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*listIter);
                /*  Check if the instruction is an activation record instruction.
                    If it is an addiu then we check the registers used. */
                if (mips_addiu == mips->get_kind()) {
                    /*  The instruction is the right type, check if the registers
                        used is the stack pointer, sp. */
                    bool isDeactivation = true;
                    /*  Get the operand list and check the operands. */
                    SgAsmExpressionPtrList& opExprList = mips->get_operandList()->get_operands();
                    /*  Check the operands. */
                    for(SgAsmExpressionPtrList::iterator opIter = opExprList.begin();
                        opIter != opExprList.end(); ++opIter) {
                        /*  Check if the expression is a registerexpression. */
                        if (V_SgAsmDirectRegisterExpression == (*opIter)->variantT()) {
                            /*  Decode the register and check it. If it is not SP then break. */
                            registerStruct regS = decodeRegister(*opIter);
                            if (sp != regS.regName) {
                                /*  The register is not sp. It is not a forbidden instruction. */
                                isDeactivation = false;
                                break;
                            }
                        }
                    }
                    /*  The instruction is an activation so it needs to be stored
                        and set as a forbidden instruction. */
                    if (true == isDeactivation) {
                        deactivationInstruction.insert(mips);
                        forbiddenInstruction.insert(mips);
                        if (debugging) {
                            std::cout << "forbidden deactivation instruction found: " << std::hex << mips->get_address() << std::endl;
                        }
                    }
                }
            }
        }
    }
}



/* Find the lowest and highest address in the function cfg */
void CFGhandler::findAddressRange() {
    /* variables for highest and lowest address, initialized
        to min and max values for the types. */
    rose_addr_t lowestAddr = std::numeric_limits<rose_addr_t>::max();
    rose_addr_t highestAddr = std::numeric_limits<rose_addr_t>::min();

    /* Debug print. */
    if (debugging) {
        std::cout << "Highest: " << std::hex << highestAddr  << " Lowest: " << lowestAddr << std::endl;
    }

    /* Go through the basic blocks and look at the first
        and last address compare to previous values and save */
    for(std::pair<CFGVIter, CFGVIter> pIter = vertices(*functionCFG);
        pIter.first != pIter.second; ++pIter.first) {
        /* get the basic block */
        SgAsmBlock* bb = get(boost::vertex_name, *functionCFG, *pIter.first);
        /* extract the statement list and check the address of the
            first and last instruction */
        SgAsmStatementPtrList* stmtList = &bb->get_statementList();
        /* Iterate over the list until the first mips instruction is found. */
        for(SgAsmStatementPtrList::iterator iter = stmtList->begin();
            iter != stmtList->end(); ++iter) {
            if ((*iter)->variantT() == V_SgAsmMipsInstruction) {
                /* the statement is an instruction, cast it */
                SgAsmMipsInstruction* mipsInst = isSgAsmMipsInstruction(*iter);
                rose_addr_t instAddr = mipsInst->get_address();
                /* check the address, is it higher than the highest or lower than the lowest?
                    If it is any of it then save it as the new highest or lowest */
                if (instAddr > highestAddr) {
                    //std::cout << "higher address found" << std::endl;
                    highestAddr = instAddr;
                } 
                if (instAddr < lowestAddr) {
                    //std::cout << "lower address found" << std::endl;
                    lowestAddr = instAddr;
                } 
            }
        }
    }
    /* The highest and lowest address in the function cfg has been found, save it*/
    addressRange.first = highestAddr;
    addressRange.second = lowestAddr;
    if (debugging) {
        std::cout << "Highest: " << std::hex << addressRange.first << " Lowest: " << addressRange.second << std::endl;
    }
}

/* Makes a cfg for a specific function */
void CFGhandler::createFunctionCFG(std::string newFunctionName) {
    /* Save the programcfg and the function name. */
    functionName = newFunctionName;
    /* New cfg variable */
    functionCFG = new CFG;
    /* track visited blocks */
    std::map<SgAsmBlock *, bool> visitedBlock;
    /* track mapping between programcfg and functioncfg vertices */
    std::map<CFG::vertex_descriptor, CFG::vertex_descriptor> vertexMap;
    /* keep track of which blocks are included in the functioncfg */
    std::map<CFG::vertex_descriptor, bool> copiedVertex;

    /*  Get the property map of the function cfg   */
    boost::property_map<CFG, boost::vertex_name_t>::type functionPropMap = get(boost::vertex_name, *functionCFG);

    /*  Go through all vertices (blocks) and find blocks belonging to the
        function that is being extracted.   */
    for(std::pair<CFGVIter, CFGVIter> verticePair = vertices(*programCFG);
        verticePair.first != verticePair.second; ++verticePair.first) {
        /*  Get the SgAsmBlock from the propertymap and check the name*/
        SgAsmBlock* basicBlock = get(boost::vertex_name, *programCFG, *verticePair.first); 
        /*  See if a vertex has been visited    */
        if (visitedBlock.find(basicBlock) == visitedBlock.end()) {
            /* Set the block as visited */
            visitedBlock.insert(std::pair<SgAsmBlock*, bool>(basicBlock, true));
            /*  Retrieve the enclosing function, i.e the function the block
                belongs to. */
            SgAsmFunction* blockFunction = basicBlock->get_enclosing_function();
            /* Check the functions name with the passed name */
            if (functionName.compare(blockFunction->get_name()) == 0) {
                /*  The block belongs to the desired function add it to the function CFG. */
                CFG::vertex_descriptor newVertex = add_vertex(*functionCFG);
                /* set the values of the property map in the new CFG */
                put(functionPropMap, newVertex, basicBlock);
                /*  Save the vertex descriptor so i know which vertexes have been copies */
                copiedVertex.insert(std::pair<CFG::vertex_descriptor, bool>(*verticePair.first , true));
                /* Add both vertices to the vertexmap */
                vertexMap.insert(std::pair<CFG::vertex_descriptor, CFG::vertex_descriptor>
                (*verticePair.first, newVertex));
            } else {
                /*  The block does not belong to a desired function, set it
                    as not copied */
                copiedVertex.insert(std::pair<CFG::vertex_descriptor, bool>(*verticePair.first, false));
            }
        }
    }
    /* All vertices have been created in the new cfg with their properties.
        Now add all edges within the program */
    for(std::pair<CFGEIter, CFGEIter> edgePair = edges(*programCFG); edgePair.first != edgePair.second;
        ++edgePair.first) {
        /* retrieve source and target vertex descriptors */
        CFG::vertex_descriptor sourceVertex = source(*edgePair.first, *programCFG);
        CFG::vertex_descriptor targetVertex = target(*edgePair.first, *programCFG);

        /*  Check if the source and target vertices have been copied.
            That means that the edge is within the program */
        if (copiedVertex[sourceVertex] && copiedVertex[targetVertex]) {
            /* create the edge between the vertices  */
            CFG::vertex_descriptor newSourceV = vertexMap[sourceVertex];
            CFG::vertex_descriptor newTargetV = vertexMap[targetVertex];
            /* Add the edge to the cfg. */
            add_edge(newSourceV, newTargetV, *functionCFG);
        }
    }

    /*  Find the entry and exit block(s). */
    findEntryAndExitBlocks();
    /* Find activation records */
    findActivationRecords();
    /* Find the address range */
    findAddressRange();
}



