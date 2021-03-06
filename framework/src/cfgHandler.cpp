/* CFG Handler */

#include "cfgHandler.hpp"

/* setup for this class */
void CFGhandler::initialize(SgProject* root) {
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


/* return the activation record pair */
std::pair<SgAsmInstruction*, SgAsmInstruction*> CFGhandler::getActivationRecord() {
    return activationPair;
}


/* Is the instruction allowed to be transformed? */
bool CFGhandler::isForbiddenInstruction(SgAsmMipsInstruction* inst) {
    /* Search through the vector for the instruction, return true if found. */
    std::vector<SgAsmInstruction*>::iterator it;
    /* use std::find to search vector, it returns a iterator to
        found value or end. */
    it = std::find(forbiddenInstruction.begin(), forbiddenInstruction.end(), inst);
    /* check if the it points to end or not */
    if (it != forbiddenInstruction.end()) {
        /* is a forbidden instruction */
        return true;
    } else {
        /* the instruction is allowed to be transformed */
        return false;
    }
}

/* Searches the function cfg for activation records.
    These are saved for later use and also added to the forbidden
    instructions map. */
void CFGhandler::findActivationRecords() {
    /* Vector for edges */ 
    std::set<CFG::vertex_descriptor> targetVertices;
    std::set<CFG::vertex_descriptor> sourceVertices;
    /* Block statement lists */
    SgAsmStatementPtrList* firstStatementList;
    SgAsmStatementPtrList* lastStatementList;
    
    for(std::pair<CFGEIter, CFGEIter> edgeIter = edges(*functionCFG);
        edgeIter.first != edgeIter.second; ++edgeIter.first) {
        /* retrieve each edges target and source and save */
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
            /* The current vertex is the first block in the instruction,
                save the vertex block as firstBlock */
            SgAsmBlock* firstBlock = get(boost::vertex_name, *functionCFG, (*iter));
            firstStatementList = &firstBlock->get_statementList();
        }
    }
    for(std::set<CFG::vertex_descriptor>::iterator iter = targetVertices.begin();
        iter != targetVertices.end(); ++iter) {
        /* Find the target vertex that is not among the source vertices.
            That vertex is the exit vertex */
        if (sourceVertices.count(*iter) == 0) {
            /* The current vertex is the last block in the instruction,
                save the vertex block as lastBlock */
            SgAsmBlock* lastBlock = get(boost::vertex_name, *functionCFG, (*iter));
            lastStatementList = &lastBlock->get_statementList();
        }
    }

    /* Go through the blocks and find the activation records.
        The instruction in question is an addiu instruction with
        sp as RD and as RS and constant  */
    for(SgAsmStatementPtrList::iterator iter = firstStatementList->begin();
        iter != firstStatementList->end(); ++iter) {
        /* decode instruction */
        SgAsmMipsInstruction* mipsInst = isSgAsmMipsInstruction(*iter);
        instructionStruct currentInst = decodeInstruction(mipsInst);
        /* check instruction */
        if (currentInst.kind == mips_addiu) {
            /* check if the rd and rs is sp */
            registerStruct destination = *currentInst.destinationRegisters.begin();
            registerStruct source = *currentInst.sourceRegisters.begin();
            if (destination.regName == sp && source.regName == sp) {
                /* the registers are correct, add the instruction to the forbidden list. */
                forbiddenInstruction.push_back(mipsInst);
                /* Add the acivation record to the pair */
                activationPair.first = mipsInst;
                /* Add the instruction to the activation instruction vector */
                activationInstruction.push_back(mipsInst);
                std::cout << "forbidden instruction found: " << std::hex << currentInst.address << std::endl;
            }
        }
    }
    /* check the last blocks instructions. */
    for(SgAsmStatementPtrList::iterator iter = lastStatementList->begin();
        iter != lastStatementList->end(); ++iter) {
        /* decode instruction */
        SgAsmMipsInstruction* mipsInst = isSgAsmMipsInstruction(*iter);
        instructionStruct currentInst = decodeInstruction(mipsInst);
        /* check instruction */
        if (currentInst.kind == mips_addiu) {
            /* check if the rd and rs is sp */
            registerStruct destination = *currentInst.destinationRegisters.begin();
            registerStruct source = *currentInst.sourceRegisters.begin();
            if (destination.regName == sp && source.regName == sp) {
                /* the registers are correct, add the instruction to the forbidden list. */
                forbiddenInstruction.push_back(mipsInst);
                /* Add the acivation record to the pair */
                activationPair.second = mipsInst;
                /* Add the instruction to the activation instruction vector */
                activationInstruction.push_back(mipsInst);
                std::cout << "forbidden instruction found: " << std::hex << currentInst.address << std::endl;
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

    std::cout << "Highest: " << std::hex << highestAddr  << " Lowest: " << lowestAddr << std::endl;

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
    std::cout << "Highest: " << std::hex << addressRange.first << " Lowest: " << addressRange.second << std::endl;
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
                /*  The block belongs to the desired function add it to
                    the function CFG. */
                CFG::vertex_descriptor newVertex = add_vertex(*functionCFG);
                /* set the values of the property map in the new CFG */
                put(functionPropMap, newVertex, basicBlock);
                /*  Save the vertex descriptor so i know which vertexes
                    have been copies */
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

    /* Find activation records */
    findActivationRecords();
    /* Find the address range */
    findAddressRange();
}



