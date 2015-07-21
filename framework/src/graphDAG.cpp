
/*  Include */
#include "graphDAG.hpp" 


/*  Constructor. */
graphDAG::graphDAG(SgAsmBlock* block) {
    /*  Save the pointer to the function cfg. */
    basicBlock = block;
}


/*  Builds the DAGs used when scheduling. */
void graphDAG::buildDAGs() {
    /*  Build the backward DAG. */
    buildBackwardDAG();
    //testing to print
    std::cout << std::endl << "graph" << std::endl;
    print_graph(*backwardDAG, get(boost::vertex_index2, *backwardDAG));
    std::cout << "graph end" << std::endl;
    /*  Build the forward DAG. */
    //TODO use the reverse graph function instead.
    buildForwardDAG();
}

/*  Constructs a DAG traversing forward. */
void graphDAG::buildForwardDAG() {

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

    boost::property_map<frameworkDAG, boost::vertex_index2_t>::type nameMap = get(boost::vertex_index2, *backwardDAG);
    int count = 0;

    //TODO need to fix so i have the first and last instruction
    //TODO as root and sink node. To guarantee that jumps are last and first instruction is never moved.

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
            /*  Set a string name to it. Can be used when printing. */
            std::stringstream strStream;
            strStream << mips->get_mnemonic() << " " << count;
            count++;
            put(nameMap, newNode, strStream.str());
            /*  Check destination registers. */
            //TODO take each destination register and get the resource name for it.
            //also check if the instructions are memory instructions (to memory);
            //or a accumulator instruction!
            for(regStructVector::iterator iter = currentInst.destinationRegisters.begin();
                iter != currentInst.destinationRegisters.end(); ++iter) {
                /*  Get the resource enum. */
                DAGresources::resourceEnum regResource = getRegisterResource((*iter).regName);
                /*  Do resource definition on it. */
                resourceDefined(&newNode, regResource);
            }

            /*  Check source registers. */
            //TODO take each register and get the resource for it.
            //also check if the instruction is a memory instruction (from memory);
        }
        
    }
    /*  The backward dag has been built. */
}


/*  Function that handles definition of resources. */
//TODO i believe i need the instruction struct here.
//TODO add argument for backward or forward. boolean? different add edge call?
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
                /*  Set bool so entries are removed after iteration of list. */
                removeUseEntries = true;
            }
    }
    /*  Remove the entry from the useBiMap if there are any. */
    if (true == removeUseEntries) {
        useBiMap.left.erase(newResource);
    }
    /*  insert the new node in the definition entry. */
    definitionMap[newResource] = *newNode;
}


/*  Resource used. */
void graphDAG::resourceUsed(DAGVertexDescriptor* newNode) {
    /*
        //TODO need to consider here that i cant have the newly made definition here. 
        if (resource is defined)
            add WAR arc between newnode and resource defined entry.
        add newnode as a uselist entry in resource uselist.
    */

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
        
