
/*  Includes */
#include "listScheduler.hpp"


/*  Constructor to be used. */
listScheduler::listScheduler(CFGhandler* passedHandler) {
    /*  Debug set as false. */
    debuging = false;
    /*  Saved the passed pointer. */
    cfgObject = passedHandler;
}


/*  Enable or disable debuging. */
void listScheduler::setDebuging(bool mode) {
    debuging = mode;
}

/*  Function that will be called when scheduling will be performed
    on blocks. */
void listScheduler::performScheduling() {
    /*  Get the cfg for the function */
    CFG* functionCFG = cfgObject->getFunctionCFG();
    /*  Iterate through the graph and schedule each block. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*functionCFG);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Get the basic block from the vertex. */
        SgAsmBlock* blockToSchedule = get(boost::vertex_name, *functionCFG, *iterPair.first);
        /*  get statement list to check size. */
        SgAsmStatementPtrList& stmtList = blockToSchedule->get_statementList();
        /*  Check if the size of the block is more than 2. If it is lower do not schedule. */
        if (2 < blockToSchedule->get_statementList().size()) {
            /*  Print block if debuging. */
            if (debuging) {
                std::cout << "Scheduling block." << std::endl;
                printBasicBlockInstructions(blockToSchedule);
                std::cout << std::endl;
            }
            /*  Call block scheduling function. */
            scheduleBlock(blockToSchedule);
        } else {
            /*  debug print. */
            if (debuging) {
                std::cout << "Skipped scheduling block due to size." << std::endl;
                printBasicBlockInstructions(blockToSchedule);
                std::cout << std::endl;
            }
        }
    }
}



/*  Handles scheduling a block according to scheduling variables.
    A DAG is built for the block and with it it is scheduled. */
void listScheduler::scheduleBlock(SgAsmBlock* basic) {
    /*  Clear the variable map. */
    variableMap.clear();

    /*  Build dag object. */
    graphDAG blockDAG(basic);
    /*  Enable debuging in DAG code if set. */
    if (debuging) {
        blockDAG.setDebuging(debuging);
    }
    /*  Let it build DAGs. */
    blockDAG.buildDAGs();

    //TODO Create a function that traverses the DAG and initializes the variable struct.
    initializeListVariables(&blockDAG);

    propagateEST(&blockDAG);
    //TODO create a function that propagates the EST values in the forward DAG.
    //TODO it will push values to its children and then visit the children,
    //TODO the children will push the values to its children and then visit its children.
    //TODO There is a possibility that a node is revisited since it could be a child to a later
    //TODO visited node.

    //TODO create a function that propagates the LST values through the backward DAG.
    //TODO it will have the same behavior as the EST function.


    //TODO create and calculate the variables used for scheduling.
    /*  Visitor object. */
    listForwardBFSVisitor forwardVariableBuilder(&variableMap, debuging);
    /*  Get forward DAG. */
    frameworkDAG* forwardDAG = blockDAG.getForwardDAG();
    /*  In the forward pass the first instruction is the root. */
    DAGVertexDescriptor* forwardRoot = blockDAG.getForwardDAGRoot();
    /*  Perform a forward pass in the DAG. */
    if (debuging) {
        std::cout << "Performing forward pass on DAG." << std::endl << std::endl;
    }
//    breadth_first_search(*forwardDAG, *forwardRoot,
//        boost::visitor(forwardVariableBuilder));

    if (debuging) {
        std::cout << std::endl;
    }

    /*  Perform a backward pass in the DAG. */
    /*  Get the backward DAG. */
    frameworkDAG* backwardDAG = blockDAG.getBackwardDAG();
    /*  In the backward pass the last instruction is the root. */
    DAGVertexDescriptor* backwardRoot = blockDAG.getBackwardDAGRoot();
    /*  Create a backwards iterator. */
    listBackwardBFSVisitor backwardVariableBuilder(&variableMap, backwardRoot, debuging);
    /*  Perform the backward pass. */
    if (debuging) {
        std::cout << "Performing backward pass on DAG." << std::endl << std::endl;
    }
    /*  Call on breadth first search algorithm. */
//    breadth_first_search(*backwardDAG, *backwardRoot,
//        boost::visitor(backwardVariableBuilder));

    //TODO do the actuall scheduling.
    
}


/*  Initialize the variableMap, make an entry for each instruction
    and set some of the variables. */
void listScheduler::initializeListVariables(graphDAG* DAGobject) {
    /*  debug printout. */
    if (debuging) {
        std::cout << "Initializing variable map." << std::endl;
    }
    /*  Get a reference to a forward DAG. */
    frameworkDAG* forwardDAG = DAGobject->getForwardDAG();
    /*  Iterate through one of the DAGs and create a initial
        entry for each instruction. */
    for(std::pair<DAGVIter, DAGVIter> iterPair = vertices(*forwardDAG);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Retrieve the isntruciton pointer. */
        SgAsmMipsInstruction* nodeInst = get(boost::vertex_name, *forwardDAG, *iterPair.first);
        /*  Create an entry for the vertice instruction.
            Also set some of the variable values. */
        instructionVariables nodeVars;
        /*  Set the execution time of the instruction. */
        //TODO need to change here if i set different values on instructions. 
        nodeVars.executionTime = 1;
        /*  Insert the variable struct into the map. */
        variableMap.insert(nodeMapPair(nodeInst, nodeVars));
    }
    /*  Debug print. */
    if (debuging) {
        std::cout << "Created " << variableMap.size() << " entries." << std::endl;
    }
}

/*  Propagates the values through the DAG. It is like a breadth first search.
    It has an additional condition though that is that nodes that have had
    new EST values assigned to them will be visited. The version therefore
    does not use coloring to mark visited. */
void listScheduler::propagateEST(graphDAG* DAGobject) {
    /*  Using a boost buffer to queue the nodes that i am visiting. */
    boost::queue<DAGVertexDescriptor> visitQueue;

    /*  Get forward DAG. */
    frameworkDAG* forwardDAG = DAGobject->getForwardDAG();
    /*  In the forward pass the first instruction is the root. */
    DAGVertexDescriptor* forwardRoot = DAGobject->getForwardDAGRoot();

    /*  Push the root vertex into the queue. */
    visitQueue.push(*forwardRoot);

    //TODO set the EST of the root node outside?
    SgAsmMipsInstruction* rootMips = get(boost::vertex_name, *forwardDAG, *forwardRoot);
    /*  Get the variables of the popped node. */
    instructionVariables rootVars = variableMap.find(rootMips)->second;
    /*  Set the EST value in the root and save it. */
    rootVars.earliestStart = 1; 
    variableMap[rootMips] = rootVars;
    /*  Debug printout. */
    if (debuging) {
        std::cout << "EST set to root node." << std::endl;
    }

    /*  BFS algorithm. Continue as long as the queue is not empty. */
    while(!visitQueue.empty()) {
        /*  Take vertex from queue. */
        DAGVertexDescriptor visitedNode = visitQueue.top();
        /*  Remove it from the queue. */
        visitQueue.pop();
        /*  Get the instruction pointer of the popped node. */
        SgAsmMipsInstruction* visitedMips = get(boost::vertex_name, *forwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedMips)->second;
        /*  Go through the out edges of the vertex. Check if the EST of
            the target vertice should be reassigned. */
        for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(visitedNode, *forwardDAG);
            edgePair.first != edgePair.second; ++edgePair.first) {
            /*  Get the target vertice of the edge and check if the ESt should
                be reassigned. */
            DAGVertexDescriptor targetNode = target(*edgePair.first, *forwardDAG);
            /*  Get the instruction pointer. */
            SgAsmMipsInstruction* targetMips = get(boost::vertex_name, *forwardDAG, targetNode);
            /*  Retrieve the variables for the target. */
            instructionVariables targetVars = variableMap.find(targetMips)->second;
            /*  Check if the EST of the target is lower than this nodes EST + Execution time. */
            if (targetVars.earliestStart < (visitedVars.earliestStart + visitedVars.executionTime)) {
                /*  The EST the visited node produces is higher than the assign so pass it over. */
                targetVars.earliestStart = visitedVars.earliestStart + visitedVars.executionTime;
                /*  Save the new EST in the mapping. */
                variableMap[targetMips] = targetVars;
                /*  Push the target node to the queue so it will be visited so it can propagate
                    its new EST. */
                visitQueue.push(targetNode);

                /*  Debug printout. */
                if (debuging) {
                    std::cout << "EST set to " << targetVars.earliestStart << std::endl;
                }
            }
        }
    }
}


/*  Function is used to propagate the LST values. It also calculates slack
    which is calculated with EST and LST. (LST - EST). */
void listScheduler::propagateLST(graphDAG* DAGobject) {
    /*  Using a boost buffer to queue the nodes that i am visiting. */
    boost::queue<DAGVertexDescriptor> visitQueue;

    /*  Get forward DAG. */
    frameworkDAG* backwardDAG = DAGobject->getBackwardDAG();
    /*  In the forward pass the first instruction is the root. */
    DAGVertexDescriptor* backwardRoot = DAGobject->getBackwardDAGRoot();

    /*  Push the root vertex into the queue. */
    visitQueue.push(*backwardRoot);

    /*  Set the correct values in the root node. */
    SgAsmMipsInstruction* rootMips = get(boost::vertex_name, *backwardDAG, *backwardRoot);
    /*  Get the variables of the popped node. */
    instructionVariables rootVars = variableMap.find(rootMips)->second;
    /*  Set the EST value in the root and save it. */
    //TODO
    //rootVars.earliestStart = 1; 
    variableMap[rootMips] = rootVars;
    /*  Debug printout. */
    if (debuging) {
        std::cout << "LST set in root node to " << std::endl;
    }

    /*  BFS algorithm. Continue as long as the queue is not empty. */
    while(!visitQueue.empty()) {
        /*  Take vertex from queue. */
        DAGVertexDescriptor visitedNode = visitQueue.top();
        /*  Remove it from the queue. */
        visitQueue.pop();
        /*  Get the instruction pointer of the popped node. */
        SgAsmMipsInstruction* visitedMips = get(boost::vertex_name, *backwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedMips)->second;
        /*  Go through the out edges of the vertex. Check if the EST of
            the target vertice should be reassigned. */
        for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(visitedNode, *backwardDAG);
            edgePair.first != edgePair.second; ++edgePair.first) {
            /*  Get the target vertice of the edge and check if the ESt should
                be reassigned. */
            DAGVertexDescriptor targetNode = target(*edgePair.first, *backwardDAG);
            /*  Get the instruction pointer. */
            SgAsmMipsInstruction* targetMips = get(boost::vertex_name, *backwardDAG, targetNode);
            /*  Retrieve the variables for the target. */
            instructionVariables targetVars = variableMap.find(targetMips)->second;
            /*  Check if the EST of the target is lower than this nodes EST + Execution time. */
            if (targetVars.earliestStart < (visitedVars.earliestStart + visitedVars.executionTime)) {
                /*  The EST the visited node produces is higher than the assign so pass it over. */
                targetVars.earliestStart = visitedVars.earliestStart + visitedVars.executionTime;
                /*  Save the new EST in the mapping. */
                variableMap[targetMips] = targetVars;
                /*  Push the target node to the queue so it will be visited so it can propagate
                    its new EST. */
                visitQueue.push(targetNode);

                /*  Debug printout. */
                if (debuging) {
                    std::cout << "LST set to " << targetVars.earliestStart << std::endl;
                }
            }
        }
    }

}



/*  Pass the variable reference. */
//void listForwardBFSVisitor::passVariableMapReference(nodeMap* ptr) {
//    /*  Save the reference. */
//    variableMapPtr = ptr;
//}


/*  When initializing the vertices in the forward pass an entry is made
    for the instruction in the container and the execution time is set. */
void listForwardBFSVisitor::initialize_vertex(DAGVertexDescriptor node, const frameworkDAG& graph) {
    /*  Get the instruction pointer from the propertymap. */
    SgAsmMipsInstruction* nodeInst = get(boost::vertex_name, graph, node); 
    /*  Debug print. */
    if (visitorDebuging) {
        instructionStruct instStruct = decodeInstruction(nodeInst);
        std::cout << "Initializing instruction: " << std::endl;
        printInstruction(nodeInst);
    }
    /*  Create a struct for the variables. */
    instructionVariables nodeVars;
    //TODO This i need to fix in case i will set different execution times of instructions. 
    nodeVars.executionTime = 1;
    /*  Insert the struct into the map. */
    variableMapPtr->insert(nodeMapPair(nodeInst, nodeVars));
}

/*  Called when the vertex is discovered. */
void listForwardBFSVisitor::discover_vertex(DAGVertexDescriptor node, const frameworkDAG& graph) {
    /*  Get the instruction pointer. */
    SgAsmMipsInstruction* nodeInst = get(boost::vertex_name, graph, node);
    /*  Retrieve the discovered vertex variables. */
    instructionVariables nodeVars = variableMapPtr->find(nodeInst)->second;
    /*  If it is the root node then the EST is -1 and should be set to 1. */
    if (-1 == nodeVars.earliestStart) {
        if (visitorDebuging) {
            std::cout << "EST set in root node." << std::endl;
        }
        nodeVars.earliestStart = 1;
        /*  Save the variables. */
        (*variableMapPtr)[nodeInst] = nodeVars;
    }
    /*  Get the out edges of the vertice and iterate over them. */
    for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(node, graph);
        edgePair.first != edgePair.second; ++edgePair.first) {
        /*  Get the target vertice and check the EST of the target vertice. */
        DAGVertexDescriptor targetNode = target(*edgePair.first, graph);
        SgAsmMipsInstruction* targetMips = get(boost::vertex_name, graph, targetNode);
        /*  Retrieve the variables for the target. */
        instructionVariables targetVars = variableMapPtr->find(targetMips)->second;
        /*  Check if the targets EST is less than this nodes plus its latency(execution time?) */
        if (targetVars.earliestStart < (nodeVars.earliestStart + nodeVars.executionTime)) {
            /*  Set the new EST. */
            targetVars.earliestStart = nodeVars.earliestStart + nodeVars.executionTime;
            if (visitorDebuging) {
                std::cout << "EST set to: " << targetVars.earliestStart << std::endl;
            }
            /*  Insert it into the map again. */
            (*variableMapPtr)[targetMips] = targetVars;
        }
    }
}

/*  Called when the vertex is discovered. */
void listBackwardBFSVisitor::discover_vertex(DAGVertexDescriptor node, const frameworkDAG& graph) {
    /*  Get the instruction pointer. */
    SgAsmMipsInstruction* nodeInst = get(boost::vertex_name, graph, node);
    /*  Retrieve the discovered vertex variables. */
    instructionVariables nodeVars = variableMapPtr->find(nodeInst)->second;
    /*  If it is the root node then the EST is -1 and should be set to 1. */
    if (node == *rootPtr) {
        /*  The latest start time for the root node (last node in forward)
            is the earliest start time. */
        nodeVars.latestStart = nodeVars.earliestStart;
        if (visitorDebuging) {
            std::cout << "LST set in root node to: " << nodeVars.latestStart << std::endl;
        }
        /*  Calculate the slack for this node as well. */
        nodeVars.slack = nodeVars.latestStart - nodeVars.earliestStart;
        /*  Save the updated variable values. */
        (*variableMapPtr)[nodeInst] = nodeVars;
    }
    /*  Get the out edges of the vertice and iterate over them. */
    for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(node, graph);
        edgePair.first != edgePair.second; ++edgePair.first) {
        /*  Get the target vertice and check the EST of the target vertice. */
        DAGVertexDescriptor targetNode = target(*edgePair.first, graph);
        SgAsmMipsInstruction* targetMips = get(boost::vertex_name, graph, targetNode);
        /*  Retrieve the variables for the target. */
        instructionVariables targetVars = variableMapPtr->find(targetMips)->second;
        /*  Check if the targets LST is less than this nodes plus its latency(execution time?) */
        if (targetVars.latestStart > (nodeVars.latestStart - nodeVars.executionTime)) {
            /*  Set the new LST. */
            targetVars.latestStart = nodeVars.latestStart - nodeVars.executionTime;
            if (visitorDebuging) {
                std::cout << "LST set to: " << targetVars.latestStart<< std::endl;
            }
            /*  Insert it into the map again. */
            (*variableMapPtr)[targetMips] = targetVars;
        }
    }
}


/*  Called when an edge is examined. */
//void listForwardBFSVisitor::examine_edge(DAGEdgeDescriptor edge, const frameworkDAG& graph) {
//    /*  From the edge i can get the target vertice. Then i can replace the EST in the
//        target node if the new one is higher than its current. */
//    DAGVertexDescriptor targetNode = target(edge, graph);
//    /*  Retrieve the entry for the target. */
//}

