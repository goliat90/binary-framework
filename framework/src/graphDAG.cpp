
/*  Include */
#include "graphDAG.hpp" 


/*  Constructor. */
graphDAG::graphDAG(SgAsmBlock* block) {
    /*  Save the pointer to the function cfg. */
    basicBlock = block;
    /*  Set debuging. */
    debuging = false;
    /*  Set instruction pointers to null. */
    lastInstruction = NULL;
    firstInstruction = NULL;
}

/*  Set debuging. */
void graphDAG::setDebuging(bool mode) {
    debuging = mode;
}

/*  Builds the DAGs used when scheduling. */
void graphDAG::buildDAGs() {
    /*  Build the backward DAG. */
    buildBackwardDAG();
    //testing to print
    if (debuging) {
        std::cout << std::endl << "graph" << std::endl;
        print_graph(*backwardDAG, get(boost::vertex_index2, *backwardDAG));
        std::cout << "graph end" << std::endl << std::endl;
    }
    /*  Build the forward DAG. */
    //TODO use the reverse graph function instead.
    buildForwardDAG();
}

/*  Constructs a DAG traversing forward. */
void graphDAG::buildForwardDAG() {
    /*  Constructed by reversing the backward DAG. */
}

/*  Constructs DAG traversing backwards. */
void graphDAG::buildBackwardDAG() {
    /*  Clear the sets. */
    definitionMap.clear();
    useBiMap.clear();
    /*  Create framework DAG object. */
    backwardDAG = new frameworkDAG;
    /*  Get the statement list. */
    SgAsmStatementPtrList& stmtList = basicBlock->get_statementList();

    /*  Property map for the vertex_name, which is an instruction pointer. */
    vertexInstructionMap instructionMap = get(boost::vertex_name, *backwardDAG);

    /*  Property map for the naming of nodes. */
    vertexIndexNameMap nameMap = get(boost::vertex_index2, *backwardDAG);
    /*  Counter to number the nodes in order of inserted. */
    int count = 0;

    /*  Check how many instructions there are. Depending on this examine
        if the last instruction is a jump. Also save the first instruction.
        1 instruction = cant change order.
        2 instructions = first cant move so only one instrution to schedule.
        3 instructions = first fixed, potential to schedule the two other if last is not a branch. */
    if (2 < stmtList.size()) {
        /*  Debug print. */
        if (debuging) {
            std::cout << "Saving the first instruction." << std::endl;
        }
        /*  Save a reference to the first instruction. */
        firstInstruction = isSgAsmMipsInstruction(stmtList.front());
        /*  Check if the last instruction is a branch instruction, if true then save
            a reference to it. */
        if (true == isBranchInstruction(stmtList.back())) {
            /*  Debug print. */
            if (debuging) {
                std::cout << "Saving last instruction, it is a branch." << std::endl;
            }
            /* The last instruction is recognized as a branch instruction. */
            lastInstruction = isSgAsmMipsInstruction(stmtList.back());
        }
    }

    /*  iterate through the statement list backwards. */
    for(SgAsmStatementPtrList::reverse_iterator rIter = stmtList.rbegin();
        rIter != stmtList.rend(); ++rIter) {
        /*  For each statement, check definitions and then use if it's
            an instruction. To do that the instruction is decoded. */
        if (V_SgAsmMipsInstruction == (*rIter)->variantT()) {
            /*  Cast to correct pointer. */
            SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*rIter);
            /*  Decode. */
            instructionStruct currentInst = decodeInstruction(mips);
            /*  Create a new node in the graph. */
            DAGVertexDescriptor newNode = add_vertex(*backwardDAG);
            /*  Check if the instruction is the first or last. */
            if ((*rIter) == firstInstruction) {
                firstInstructionVertice = newNode;
            } else if ((*rIter) == lastInstruction) {
                lastInstructionVertice = newNode;
            }

            /*  Set a string name to it. Can be used when printing. */
            std::stringstream strStream;
            strStream << mips->get_mnemonic() << " " << count;
            count++;
            /*  Save the string as a property of the node. */
            put(nameMap, newNode, strStream.str());
            /*  Save the instruction pointer as a property of the vertex. */
            put(instructionMap, newNode, mips);

            /*  debug print. */
            if (debuging) {
                std::cout << "--------------------" << std::endl;
            }
            /*  Check destination registers. */
            for(regStructVector::iterator iter = currentInst.destinationRegisters.begin();
                iter != currentInst.destinationRegisters.end(); ++iter) {
                /*  Debug print. */
                if (debuging) {
                    std::cout << "Definition check for reg: " << getRegisterString((*iter).regName) << std::endl;
                }
                /*  Get the resource enum. */
                DAGresources::resourceEnum regResource = getRegisterResource((*iter).regName);
                /*  Do resource definition on it. */
                resourceDefined(&newNode, regResource);
            }
            /*  If the instruction reads or write to accumulator register
                it is handled here. */
            if (true == involvesAcc(currentInst.kind)) {
                /*  Debug print. */
                if (debuging) {
                    std::cout << "Instruction defines or uses accumulator. " << std::endl;
                }
                /*  The instruction writes or reads to the accumulator register.
                    Handle the resource. */
                manageAccumulator(&newNode, currentInst.kind);
            }

            /*  Check source registers. */
            for(regStructVector::iterator iter = currentInst.sourceRegisters.begin();
                iter != currentInst.sourceRegisters.end(); ++iter) {
                /*  Debug print. */
                if (debuging) {
                    std::cout << "Use check for reg: " << getRegisterString((*iter).regName) << std::endl;
                }
                /*  Get the resource enum. */
                DAGresources::resourceEnum regResource = getRegisterResource((*iter).regName);
                /*  Do resource used on it. */
                resourceUsed(&newNode, regResource);
            }

            /*  Check the format the instruction has. If it is a format for 
                store or load instructions we hande the memory resource. */
            if (I_RS_MEM_RT_C == currentInst.format) {
                /*  degbug print. */
                if (debuging) {
                    std::cout << strStream.str() << " writes to memory" << std::endl;
                }
                /*  store instruction. Writing to memory. can be called defining. */
                resourceDefined(&newNode, DAGresources::memoryResource);
            } else if (I_RD_MEM_RS_C == currentInst.format) {
                /*  degbug print. */
                if (debuging) {
                    std::cout << strStream.str() << " reads from memory" << std::endl;
                }
                /*  load instruction. This means we are reading from memory or using it. */
                resourceUsed(&newNode, DAGresources::memoryResource);
            }
            if (debuging) {
                std::cout << "--------------------" << std::endl;
            }
        }
    }
    /*  The backward dag has been built. Now ensure that the last
        instruction will be a jump if there is one in the block.
        Also ensure that the first instruction in the block will not be moved. */
    if (NULL != firstInstruction) {
        /*  There is a reference to the first instruction. Go through the 
            graph and connect nodes without in edges to the first instructions
            node. This is to make sure the first instruction will not be moved. */
        /*  Set to store the vertices that do not have any in-edges. */
        std::set<DAGVertexDescriptor> startVertices;
        for(std::pair<DAGVIter, DAGVIter> iterPair = vertices(*backwardDAG);
            iterPair.first != iterPair.second; ++iterPair.first) {
            /*  Check if the vertice has any in edges. */
            if (0 == out_degree(*iterPair.first, *backwardDAG)) {
                /*  debug print. */
                if (debuging) {
                    std::cout << get(boost::vertex_index2, *backwardDAG, *iterPair.first)
                        << " has no out-edges." << std::endl;
                }
                /*  Save the vertice. */
                startVertices.insert(*iterPair.first);
            }
        }
        /*  Go through the vertice set and add an edge between the vertice for
            the first instruction and the vertices in the set. */
        for(std::set<DAGVertexDescriptor>::iterator vIter = startVertices.begin();
            vIter != startVertices.end(); ++vIter) {
            /*  Add an edge between the vertice and the first instruction vertice. */
            //add_edge(*vIter, firstInstructionVertice , *backwardDAG);
        }
    }
}


/*  Function that handles definition of resources. */
//TODO i believe i need the instruction struct here.
//TODO add argument for backward or forward. boolean? different add edge call?
//TODO there is a function in boost that reverses all the edges of the graph.
void graphDAG::resourceDefined(DAGVertexDescriptor* newNode, DAGresources::resourceEnum newResource) {
    /*
        if (resource is defined AND no use of the resource)
            add WAW arc between newnode and resource defined entry (previous instruction);

        for each (use definition entry in resource uselist in ascending order) do
        {
            add RAW arc between newnode and uselist entry
            remove remove uselist entry from resource uselist.
        }
        insert newnode as the resource definition entry.
    */
    
    /*  Check if the resource (register or memory) is defined AND
        if there is no use of the resource. */
    if (1 == definitionMap.count(newResource) && (0 == useBiMap.left.count(newResource))) {
        /*  Get the previous vertex defining the resource. */
        DAGVertexDescriptor lastDefinitionVertex = definitionMap.find(newResource)->second;
        /*  ADD WAW arc between latest definition(created node) and last definition,. */
        //TODO do i need to distinguish the types of arcs?
        //TODO could be that i need to set the latency values here. 
        add_edge(lastDefinitionVertex, *newNode, *backwardDAG);
        /*  debug printout. */
        if (debuging) {
            std::cout << "adding WAW edge." << std::endl;
        }
    }

    /*  for each node using the definition. */
    bool removeUseEntries = false;
    for(useContainer::left_iterator useIter = useBiMap.left.begin();
        useIter != useBiMap.left.end(); ++useIter) {
        //TODO do i need to distinguish the types of arcs?
        //TODO could be that i need to set the latency values here. 
        /*  Check if the entry uses the new resource. */
        if (newResource == useIter->first) {
            /*  add RAW arc between that node and the newnode. */
            //TODO do i need to distinguish the types of arcs?
            //TODO could be that i need to set the latency values here. 
            add_edge(useIter->second, *newNode, *backwardDAG);
            if (debuging) {
                std::cout << "Adding RAW edge." << std::endl;
            }
            /*  Set bool so entries are removed after iteration of list. */
            removeUseEntries = true;
        }
    }
    /*  Remove the entry from the useBiMap if there are any. */
    if (true == removeUseEntries) {
        int entries = useBiMap.left.erase(newResource);
        if (debuging) {
            std::cout << entries << " removed." << std::endl;
        }
    }
    /*  insert the new node in the definition entry. */
    definitionMap[newResource] = *newNode;
}



/*  Resource used. */
void graphDAG::resourceUsed(DAGVertexDescriptor* newNode, DAGresources::resourceEnum newResource) {
    /*
        //TODO need to consider here that i cant have the newly made definition here. 
        if (resource is defined)
            add WAR arc between newnode and resource defined entry.
        add newnode as a uselist entry in resource uselist.
    */
    if (1 == definitionMap.count(newResource)) {
        /*  Add WAR arc between newnode and defined node. */
        /*  Get the node that is defining the resource. */
        DAGVertexDescriptor definitionNode = definitionMap.find(newResource)->second;
        //TODO here do a check so target and source node is not the same.
        //TODO will that work?
        if (definitionNode != *newNode) {
            /*  The nodes are not the same, create the edge. */
            add_edge(definitionNode, *newNode, *backwardDAG);
        }
        if (debuging) {
            std::cout << "adding WAR edge" << std::endl;
        }
    }
    /*  Insert the new node in the use list. */
    useBiMap.insert(useContainer::value_type(newResource, *newNode));
}


/*  This function handles the definition and use of the accumulator register. */
void graphDAG::manageAccumulator(DAGVertexDescriptor* newNode, MipsInstructionKind instKind) {
    /*  Depending on the kind of the resource make appropriate calls
        to the resourceDefined and resourceUsed functions. */
    switch(instKind) {
        case mips_div:
        case mips_divu:
        case mips_madd:
        case mips_maddu:
        case mips_msub:
        case mips_msubu:
        case mips_mult:
        case mips_multu:{
            /*  All the above instructions write to the accumulator register.
                Call on resourceDefined with Hi and Low as resources. */
            resourceDefined(newNode, DAGresources::highAcc);
            resourceDefined(newNode, DAGresources::lowAcc);
            break;
        }
        case mips_mfhi:{
            /* Reads high to a register. */
            resourceUsed(newNode, DAGresources::highAcc);
            break;
        }
        case mips_mflo:{
            /* Reads low to a register. */
            resourceUsed(newNode, DAGresources::highAcc);
            break;
        }
        case mips_mthi:{
            /*  Writes to the high register. */
            resourceDefined(newNode, DAGresources::highAcc);
            break;
        }
        case mips_mtlo: {
            /*  Writes to the low register. */
            resourceDefined(newNode, DAGresources::lowAcc);
            break;
        }
    }
}

/*  Check if an instruction is a branch. */
bool graphDAG::isBranchInstruction(SgAsmStatement* inst) {
    /*  Check if it is a mips instruction and cast it. */
    if (V_SgAsmMipsInstruction == inst->variantT()) {
        SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(inst);
        /*  Check if the instruction kind matches a branch instruction. */
        switch(mips->get_kind()) {
            case mips_j:
            case mips_jal:
            case mips_jalr:
            case mips_beq:
            case mips_bne:
            case mips_bgez:
            case mips_bgezal:
            case mips_bgtz:
            case mips_blez:
            case mips_bltz:
            return true;
        }
    }
    /*  It was not an instruction or not a branch instruction. */
    return false;
}

/*  Check if the instructionkind matches that of an instruction using the accumulator. */
bool graphDAG::involvesAcc(MipsInstructionKind kind) {
    /* If the instruction reads or writes to acc then return true. */
    switch(kind) {
        case mips_div:
        case mips_divu:
        case mips_madd:
        case mips_maddu:
        case mips_msub:
        case mips_msubu:
        case mips_mult:
        case mips_multu:
        /*  Acc move instructions. */
        case mips_mfhi:
        case mips_mflo:
        case mips_mthi:
        case mips_mtlo:
            return true;
        default:
            return false;
    }
}


/*  Help function to get correct resourse name. */
DAGresources::resourceEnum graphDAG::getRegisterResource(mipsRegisterName reg) {
    /* Variable. */
    DAGresources::resourceEnum resource;
    
    /*  Set the correct resource on the register. */
    switch(reg) {
        case zero: resource = DAGresources::zero;
            break;
        case at: resource = DAGresources::at;
            break;
        case v0: resource = DAGresources::v0;
            break;
        case v1: resource = DAGresources::v1;
            break;

        case a0: resource = DAGresources::a0;
            break;
        case a1: resource = DAGresources::a1;
            break;
        case a2: resource = DAGresources::a2;
            break;
        case a3: resource = DAGresources::a3;
            break;
                                            
        case t0: resource = DAGresources::t0;
            break;
        case t1: resource = DAGresources::t1;
            break;
        case t2: resource = DAGresources::t2;
            break;
        case t3: resource = DAGresources::t3;
            break;
        case t4: resource = DAGresources::t4;
            break;
        case t6: resource = DAGresources::t6;
            break;
        case t7: resource = DAGresources::t7;
            break;
                                            
        case s0: resource = DAGresources::s0;
            break;
        case s1: resource = DAGresources::s1;
            break;
        case s2: resource = DAGresources::s2;
            break;
        case s3: resource = DAGresources::s3;
            break;
        case s4: resource = DAGresources::s4;
            break;
        case s5: resource = DAGresources::s5;
            break;
        case s6: resource = DAGresources::s6;
            break;
        case s7: resource = DAGresources::s7;
            break;
                                            
        case t8: resource = DAGresources::t8;
            break;
        case t9: resource = DAGresources::t9;
            break;
                                            
        case k0: resource = DAGresources::k0;
            break;
        case k1: resource = DAGresources::k1;
            break;
                                            
        case gp: resource = DAGresources::gp;
            break;
        case sp: resource = DAGresources::sp;
            break;
        case fp: resource = DAGresources::fp;
            break;
        case ra: resource = DAGresources::ra;
            break;
    }
    /*  return the resource. */
    return resource;
}
        
