
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
    /*  Clear the nodenumber map as well. */
    nodeNumberToVertex.clear();

    /*  Build dag object. */
    graphDAG blockDAG(basic);
    /*  Enable debuging in DAG code if set. */
    if (debuging) {
        blockDAG.setDebuging(debuging);
    }
    /*  Let it build DAGs. */
    blockDAG.buildDAGs();


    if (debuging) {
        std::cout << "----------------------------------------" << std::endl;
    }
    /*  Initialize the variables for each instruction of the list scheduler. */
    initializeListVariables(&blockDAG);

    /*  Perform a forward pass in the forward dag and propagate the EST values. */
    if (debuging) {
        std::cout << "----------------------------------------" << std::endl;
    }
    propagateEST(&blockDAG);

    /*  Perform a pass on on the backward dag to propagate the LST values. */
    if (debuging) {
        std::cout << "----------------------------------------" << std::endl;
    }
    propagateLST(&blockDAG);
    
    /*  Calculate maximum delay to leaf. */
    if (debuging) {
        std::cout << "----------------------------------------" << std::endl;
    }
    maximumDelayToLeaf(&blockDAG);
    
    /*  Calculate the slack variable. */
    if (debuging) {
        std::cout << "----------------------------------------" << std::endl;
    }
    calculateSlack(&blockDAG);

    if (debuging) {
        std::cout << "----------------------------------------" << std::endl
                << "Variable calculations complete. Scheduling block." << std::endl;
    }
    /*  Schedule order list, contains the nodenumbers of the instructions. */
    std::list<int> instructionOrder;
    /*  Schedule the block. */
    forwardListScheduling(&blockDAG, instructionOrder);

    /*  Create a new statement list for the scheduled instruction order and
        replace the old instruction order in the block. */
    SgAsmStatementPtrList oldOrder = basic->get_statementList();
    /*  Frameword dag reference. */
    frameworkDAG* forwardDAG = blockDAG.getForwardDAG();
    /*  List scheduler list, populate it and then swap with oldOrder list. */
    SgAsmStatementPtrList listOrder;
    /*  Iterate through the instruction order list and get the instructions. */
    for(std::list<int>::iterator instIter = instructionOrder.begin();
        instIter != instructionOrder.end(); ++instIter) {
        /*  Get the node from the node number to vertex mapping. */
        DAGVertexDescriptor node = nodeNumberToVertex.find(*instIter)->second;
        /*  Get the instruction pointer. */
        SgAsmMipsInstruction* mipsNode = get(boost::vertex_name, *forwardDAG, node);
        /*  Push the instruction pointer to the list. */
        listOrder.push_back(mipsNode);
    }

    /*  Swap the new instruction order with the old. */
    oldOrder.swap(listOrder);
    /*  If debuging print the block. */
    if (debuging) {
        std::cout << "Instruction order for block from list scheduler." << std::endl;
        printBasicBlockInstructions(basic);
    }
}


/*  Perform list scheduling. */
void listScheduler::forwardListScheduling(graphDAG* DAGobject, std::list<int>& scheduleOrder) {
    /*  Variable declarations. */
    int listCycle = 0;
    /*  Reference to forward DAG. */
    frameworkDAG* fDAG = DAGobject->getForwardDAG();
    /*  Instructions ready for scheduling, ordered by priority.
        A priority queue could be considered but it can not be searched for duplicates. */
    std::list<instructionVariables> readyList;
    /*  Flightlist contains the instructionvariables and the cycle it was scheduled.
        This map will need to be changed if multi-issuing would be possible. */
    std::map<int, instructionVariables> inFlightList;

    /* Initialize the readylist with the root vertex variables. */
    DAGVertexDescriptor* root = DAGobject->getForwardDAGRoot();
    /*  Get the roots number. */
    int rootNumber =get(boost::vertex_index1, *fDAG, *root);
    /*  Push the struct to the variable list. */
    readyList.push_front(variableMap.find(rootNumber)->second);

    /*  Debug printout. */
    if (debuging) {
        std::cout << "Starting scheduling." << std::endl;
    }
    /*  While loops. */
    while (!readyList.empty() || !inFlightList.empty()) {
        /*  Sort the readyList according to priority with the priority compare. */
        readyList.sort(priorityCompareFunctor());
        /*  Find highest priority instruction that is ready and schedule it. */

        //TODO change this to just checking in the readylist is empty and take the
        //TODO highest prioritised instruciton. 
        for(std::list<instructionVariables>::iterator readyIter = readyList.begin();
            readyIter != readyList.end(); ++readyIter) {
            /*  Schedule the highest priority instruction. Also check if
                any of the child instructions can be scheduled. */

            //TODO for now i add the first instruction to the inflight list and break the for loop.
            inFlightList.insert(std::pair<int, instructionVariables>(listCycle, *readyIter));

            /*  Get the vertex descriptor. */
            DAGVertexDescriptor currentNode = nodeNumberToVertex.find((*readyIter).nodeNumber)->second;
            /*  Iterate over the out edges and check if the child nodes of the instruction
                can be scheduled. */
            for(std::pair<DAGOEIter, DAGOEIter> outEdgeIter = out_edges(currentNode, *fDAG);
                outEdgeIter.first != outEdgeIter.second; ++outEdgeIter.first) {
                /*  Variable used to track if a child node can be added to the readylist.
                    Which means it is ready for scheduling and execution. */
                bool childReady = true;
                /*  Get the child vertex. */
                DAGVertexDescriptor childNode = target(*outEdgeIter.first, *fDAG);
                /*  Iterate over the child in-edges and check if the parents have been
                    scheduled or if the only dependencies are WAR. */
                for(std::pair<DAGIEIter, DAGIEIter> inEdgeIter = in_edges(childNode, *fDAG);
                    inEdgeIter.first != inEdgeIter.second; ++inEdgeIter.first) {
                    /*  Check if parents are scheduled. If they are not then the
                        dependency must be a WAR dependency for it to be acceptable.
                        If these above criterias are not meet then the child can not be scheduled yet. */
                    /*  Get the parent node. */
                    DAGVertexDescriptor parentNode = source(*inEdgeIter.first, *fDAG);
                    /*  Get the node number of the parent. */
                    int parentNumber = get(boost::vertex_index1, *fDAG, parentNode); 
                    /*  Check if the parent has been scheduled or not. */
                    if (scheduleOrder.end() == std::find(scheduleOrder.begin(), scheduleOrder.end(), parentNumber)) {
                        /*  The parent has not been scheduled. Check if the dependency
                            is a WAR, if so it is still okay. */
                        edgeDependency::edgeType childDependency = get(boost::edge_weight, *fDAG, *inEdgeIter.first);
                        /*  Check if the edge property is WAR. Also if the dependency is to enforce
                            root and sink nodes it is still okay since then there are no data dependencies. */
                        if ((edgeDependency::WAR != childDependency) ||
                            (edgeDependency::sinkRootDependency == childDependency)) {
                            /*  The dependency is not WAR so the child is not ready. */
                            childReady = false;
                            /*  Break the loop. */
                            break;
                        }
                    }
                }
                /*  Add the child node to the readylist if it is ready. */
                if (true == childReady) {
                    /*  Get the child nodes number to retrieve the struct. */
                    int childNumber = get(boost::vertex_index1, *fDAG, childNode);
                    /*  Get the variable struct from the map. */
                    instructionVariables childVars = variableMap.find(childNumber)->second;
                    /*  Insert it into the ready list. If it is not already in the list. */
                    if (readyList.end() == std::find(readyList.begin(), readyList.end(), childVars)) {
                        /*  Child is inserted. */
                        readyList.push_back(childVars);
                    } else if (debuging) {
                        std::cout << "Node already in readylist." << std::endl;
                    }
                }
            }
            readyList.erase(readyIter);
            break;
        }

        /*  Increment the scheduling cycle. */
        listCycle += 1;
        if (debuging) {
            std::cout << "Incrementing cycle." << std::endl;
        }

        //TODO consider changing the for too a while loop and get a "safer"/"correct" way
        //TODO of iterating and potentially deleting elements.
        for(std::map<int, instructionVariables>::iterator flightIter = inFlightList.begin();
            flightIter != inFlightList.end(); ++flightIter) {
            /*  Debug print. */
//            if (debuging) {
//                std::cout << "Checking inflight list for finished instructions." << std::endl;
//            }
            /*  Check each instruction if they have finished this cycle.
                This is done by checking if the cycle when put inflight list plus its
                execution time is equal to the current cycle. */
            if (listCycle == (flightIter->first + flightIter->second.executionTime)) {
                /*  Debug printout. */
//                if (debuging) {
//                    std::cout << "Found instruction that has finished execution." << std::endl;
//                }
                /*  The Instruction is finished this cycle. Add it to the schedule order list. */
                DAGVertexDescriptor completedNode = nodeNumberToVertex.find(flightIter->second.nodeNumber)->second;
                /*  Add the node number to the schedule list. */
                scheduleOrder.push_back(flightIter->second.nodeNumber);

                /*  Check the child nodes of the completted instruction and see
                    if they can be scheduled. */
                for(std::pair<DAGOEIter, DAGOEIter> outEdgeIter = out_edges(completedNode, *fDAG);
                    outEdgeIter.first != outEdgeIter.second; ++outEdgeIter.first) {
                    /*  Variable determening if a child node can be scheduled. */
                    bool childScheduable = true;
                    /*  Get the target node and check its in-edges. If all their
                        dependencies are satisfied then push child to the readylist. */
                    DAGVertexDescriptor childNode = target(*outEdgeIter.first, *fDAG);
                    /*  Iteration over the in-edges. Checking that dependencies are satisfied. */
                    for(std::pair<DAGIEIter, DAGIEIter> inEdgeIter = in_edges(childNode, *fDAG);
                        inEdgeIter.first != inEdgeIter.second; ++inEdgeIter.first) {
                        /*  If all dependencies are satisfied the instruction can be put in the 
                            ready-list. First check if the parent instructions has been scheduled. */
                        //TODO how could i take forwarding into account? 
                        DAGVertexDescriptor parentNode = source(*inEdgeIter.first, *fDAG);
                        /*  Get parent node number. */
                        int parentNumber = get(boost::vertex_index1, *fDAG, parentNode);
                        /*  Check if parent is scheduled. If it has not been the we can
                            not schedule the child and we break the loop. */
                        if (scheduleOrder.end() == std::find(scheduleOrder.begin(), scheduleOrder.end(), parentNumber)) {
                           childScheduable = false; 
                           break;
                        }
                    }
                    /*  If the childs parents have all been scheduled it can
                        be added to the ready list. */
                    if (true == childScheduable) {
                        /*  Get the child number. */
                        int childNumber = get(boost::vertex_index1, *fDAG, childNode);
                        /*  Get the variable struct from the map. */
                        instructionVariables childVars = variableMap.find(childNumber)->second;
                        /*  Insert it into the ready list. If it is not already in the list. */
                        if (readyList.end() == std::find(readyList.begin(), readyList.end(), childVars)) {
                            /*  Child is inserted. */
                            readyList.push_back(childVars);
                        } else if (debuging) {
                            std::cout << "Node already in readylist." << std::endl;
                        }
                    }
                }
                /*  Remove the instruction from the inflight list. */
                //TODO this might now work. Or it works but is not a good solution.
                inFlightList.erase(flightIter);
            }
        }
    }

    /*  Debug printout. */
    if (debuging) {
        std::cout << "Scheduling complete." << std::endl;
    }

/*
    Cycle = 0
    ready-list = root nodes in DPG (Data precedence graph), This will probably be the
    a kind of list containing nodes found in the cfg to be ready to be scheduled.
    inflight-list =  instructions that are executing. 

    while (ready-list or inflight-list is not empty)
        for all instructions in ready-list in descending priority order
        //TODO The for all here might not be needed since only one instruction can be scheduled at a time.
            if a functional unit exists so a instruction can be scheduled at this cycle.
                remove instruction from ready-list and add it to inflight-list.
                add instruction to the schedule at time cycle.
                if the instruction has out-going edges.(anti-edges)
                    add the targets of the edges to the ready-list
                    //TODO this is probably WAR dependencies, after this the instruction is issued
                    //TODO and instruction writing to the same register can execute since the value has been read.
                endif
            endif
        endfor

        cycle = cycle + 1
        for all instructions in the inflight-list
            if operation finished at time cycle
                remove instruction from inflight-list
                check for nodes waiting for instruction to finish and add to ready-list
                    if all operands are available.
                    //TODO This is probably RAW, instruction needs the computed value written to memory.
                    //TODO Could probably be WAW also, write to the same register that has been written to.
            endif
        endfor
    endwhile.
*/
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
        /*  Retrieve the instruction pointer. */
        SgAsmMipsInstruction* nodeInst = get(boost::vertex_name, *forwardDAG, *iterPair.first);
        /*  Create an entry for the vertice instruction.
            Also set some of the variable values. */
        instructionVariables nodeVars;
        /*  Set the execution time of the instruction. */
        //TODO need to change here if i set different values on instructions. 
        nodeVars.executionTime = getInstructionExecutionTime(nodeInst->get_kind());
        /*  Save the node itself as well for later reference when scheduling. */
        nodeVars.nodeNumber = get(boost::vertex_index1, *forwardDAG, *iterPair.first);
        /*  Create a mapping between node number and the vertex descriptor. */
        nodeNumberToVertex.insert(numberToVertexPair(nodeVars.nodeNumber, *iterPair.first));
        /*  Insert the variable struct into the map. */
        variableMap.insert(nodeVarsPair(nodeVars.nodeNumber, nodeVars));
    }
    /*  Debug print. */
    if (debuging) {
        /*  These maps should be equal in size. */
        std::cout << "Created " << variableMap.size() << " entries in variableMap." << std::endl << std::endl;
        std::cout << "Created " << nodeNumberToVertex.size() << " entries in nodeNumberToVertex." << std::endl << std::endl;
    }
}

/*  Propagates the values through the DAG. It is like a breadth first search.
    It has an additional condition though that is that nodes that have had
    new EST values assigned to them will be visited. The version therefore
    does not use coloring to mark visited. */
void listScheduler::propagateEST(graphDAG* DAGobject) {
    /*  Debug print. */
    if (debuging) {
        std::cout << "Performing forward pass on DAG to calculate EST." << std::endl;
    }
    /*  Using a boost buffer to queue the nodes that i am visiting. */
    boost::queue<DAGVertexDescriptor> visitQueue;

    /*  Get forward DAG. */
    frameworkDAG* forwardDAG = DAGobject->getForwardDAG();
    /*  In the forward pass the first instruction is the root. */
    DAGVertexDescriptor* forwardRoot = DAGobject->getForwardDAGRoot();
    /*  Get the root nodes number. */
    int rootNumber = get(boost::vertex_index1, *forwardDAG, *forwardRoot);

    /*  Push the root vertex into the queue. */
    visitQueue.push(*forwardRoot);

    /*  Set the EST in the root node before traversal. */
    /*  Get the variables of the popped node. */
    instructionVariables rootVars = variableMap.find(rootNumber)->second;
    /*  Set the EST value in the root and save it. */
    rootVars.earliestStart = 1; 
    variableMap[rootNumber] = rootVars;
    /*  Debug printout. */
    if (debuging) {
        std::cout << "EST set to " << rootVars.earliestStart << " in root node."  << std::endl << std::endl;
    }

    /*  BFS algorithm. Continue as long as the queue is not empty. */
    while(!visitQueue.empty()) {
        /*  Take vertex from queue. */
        DAGVertexDescriptor visitedNode = visitQueue.top();
        /*  Remove it from the queue. */
        visitQueue.pop();
        /*  Get the node number of the visited node. */
        int visitedNodeNumber = get(boost::vertex_index1, *forwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedNodeNumber)->second;
        /*  Go through the out edges of the vertex. Check if the EST of
            the target vertice should be reassigned. */
        for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(visitedNode, *forwardDAG);
            edgePair.first != edgePair.second; ++edgePair.first) {
            /*  Get the target vertice of the edge and check if the ESt should
                be reassigned. */
            DAGVertexDescriptor targetNode = target(*edgePair.first, *forwardDAG);
            /*  Get the target nodes number. */
            int targetNodeNumber = get(boost::vertex_index1, *forwardDAG, targetNode);
            /*  Retrieve the variables for the target. */
            instructionVariables targetVars = variableMap.find(targetNodeNumber)->second;
            /*  Check if the EST of the target is lower than this nodes EST + Execution time. */
            if (targetVars.earliestStart < (visitedVars.earliestStart + visitedVars.executionTime)) {
                /*  The EST the visited node produces is higher than the assign so pass it over. */
                targetVars.earliestStart = visitedVars.earliestStart + visitedVars.executionTime;
                /*  Save the new EST in the mapping. */
                variableMap[targetNodeNumber] = targetVars;
                /*  Push the target node to the queue so it will be visited so it can propagate
                    its new EST. */
                visitQueue.push(targetNode);
                /*  Debug printout. */
                if (debuging) {
                    /*  Get the instruction pointer. */
                    SgAsmMipsInstruction* targetMips = get(boost::vertex_name, *forwardDAG, targetNode);
                    /*  Printout */
                    std::cout << "EST set to " << targetVars.earliestStart 
                        << " in instruction" << std::endl;
                    printInstruction(targetMips);
                    std::cout << std::endl;
                }
            }
        }
    }
}


/*  Function is used to propagate the LST values. */
void listScheduler::propagateLST(graphDAG* DAGobject) {
    /*  Perform the backward pass. */
    if (debuging) {
        std::cout << "Performing backward pass on DAG to calculate LST." << std::endl << std::endl;
    }
    /*  Using a boost buffer to queue the nodes that i am visiting. */
    boost::queue<DAGVertexDescriptor> visitQueue;
    /*  Get forward DAG. */
    frameworkDAG* backwardDAG = DAGobject->getBackwardDAG();
    /*  In the forward pass the first instruction is the root. */
    DAGVertexDescriptor* backwardRoot = DAGobject->getBackwardDAGRoot();
    /*  Get the backward nodes index number. */
    int backwardRootNumber = get(boost::vertex_index1, *backwardDAG, *backwardRoot);

    /*  Push the root vertex into the queue. */
    visitQueue.push(*backwardRoot);

    /*  Set the correct values in the root node. */
    instructionVariables rootVars = variableMap.find(backwardRootNumber)->second;
    /*  Set the LST value in the root and save it, the LST in the root
        which is the last instruction in the block is the EST. */
    rootVars.latestStart = rootVars.earliestStart; 
    variableMap[backwardRootNumber] = rootVars;
    /*  Debug printout. */
    if (debuging) {
        std::cout << "LST set in root node to " << rootVars.latestStart << std::endl << std::endl;
    }

    /*  BFS algorithm. Continue as long as the queue is not empty. */
    while(!visitQueue.empty()) {
        /*  Take vertex from queue. */
        DAGVertexDescriptor visitedNode = visitQueue.top();
        /*  Remove it from the queue. */
        visitQueue.pop();
        /*  Get the visited node number. */
        int visitedNodeNumber = get(boost::vertex_index1, *backwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedNodeNumber)->second;
        /*  Go through the out edges of the vertex. Check if the EST of
            the target vertice should be reassigned. */
        for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(visitedNode, *backwardDAG);
            edgePair.first != edgePair.second; ++edgePair.first) {
            /*  Get the target vertice of the edge and check if the EST should
                be reassigned. */
            DAGVertexDescriptor targetNode = target(*edgePair.first, *backwardDAG);
            /*  Get target nodes number. */
            int targetNodeNumber = get(boost::vertex_index1, *backwardDAG, targetNode);
            /*  Retrieve the variables for the target. */
            instructionVariables targetVars = variableMap.find(targetNodeNumber)->second;
            /*  Check if the EST of the target is lower than this nodes EST + Execution time. */
            if (targetVars.latestStart > (visitedVars.latestStart - visitedVars.executionTime)) {
                /*  The EST the visited node produces is higher than the assign so pass it over. */
                targetVars.latestStart = visitedVars.latestStart - visitedVars.executionTime;
                /*  Save the new EST in the mapping. */
                variableMap[targetNodeNumber] = targetVars;
                /*  Push the target node to the queue so it will be visited so it can propagate
                    its new EST. */
                visitQueue.push(targetNode);
                /*  Debug printout. */
                if (debuging) {
                    /*  Get the instruction pointer. */
                    SgAsmMipsInstruction* targetMips = get(boost::vertex_name, *backwardDAG, targetNode);
                    /*  Printout. */
                    std::cout << "LST set to " << targetVars.latestStart
                        << " in instruction." << std::endl;
                        printInstruction(targetMips);
                        std::cout << std::endl;
                }
            }
        }
    }
}


/*  Performs a backward pass and calculates the maximum delay to the leaf.
    The way it is done is the same as with the LST. Start from the root node
    and propagate the values. */
void listScheduler::maximumDelayToLeaf(graphDAG* DAGobject) {
/*
    for each node in basic block in reverse order.
        node->maxlength = 0
        for each child of the node.
            the maximum delay is the max of the 
                childs maximum delay or the current node + execution time.

    !!: The maximum delay will propagate from the last node up to the first.
*/
    /*  Perform the backward pass. */
    if (debuging) {
        std::cout << "Performing backward pass on DAG to calculate maximum delay." << std::endl << std::endl;
    }
    /*  Using a boost buffer to queue the nodes that i am visiting. */
    boost::queue<DAGVertexDescriptor> visitQueue;
    /*  Get backward DAG. */
    frameworkDAG* backwardDAG = DAGobject->getBackwardDAG();
    /*  In the backward pass the last instruction is the root. */
    DAGVertexDescriptor* backwardRoot = DAGobject->getBackwardDAGRoot();
    /*  Get the root node number. */
    int rootNodeNumber = get(boost::vertex_index1, *backwardDAG, *backwardRoot);

    /*  Push the root vertex into the queue. */
    visitQueue.push(*backwardRoot);

    /*  Set the correct values in the root node. */
    instructionVariables rootVars = variableMap.find(rootNodeNumber)->second;
    /*  Set the delay value of the root, since the root is the last instruction
        it has no delay because there is no instruction after it. 
    //TODO need to consider this or if i should have execution time here. */
    rootVars.maximumDelayToLeaf = 0;
    /*  Debug print. */
    if (debuging) {
        std::cout << "maximum delay to leaf set in root to " << rootVars.maximumDelayToLeaf
            << std::endl;
    }
    /*  Save the variable values. */
    variableMap[rootNodeNumber] = rootVars;

    /*  Traverse the DAG in a BFS maner, It will revisit nodes that have their
        values updated though, a deviation from normal BFS. */
    while(!visitQueue.empty()) {
        /*  Pop node from the queue. */
        DAGVertexDescriptor visitedNode = visitQueue.top();
        /*  Remove it from the queue. */
        visitQueue.pop();
        /*  Get visited nodes number. */
        int visitedNodeNumber = get(boost::vertex_index1, *backwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedNodeNumber)->second;
        /*  Iterate over the out edges and check the maximum delay to leaf
            of the nodes children. */
        for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(visitedNode, *backwardDAG);
            edgePair.first != edgePair.second; ++edgePair.first) {
            /*  Get the target/child node of the currently visited node. */
            DAGVertexDescriptor targetNode = target(*edgePair.first, *backwardDAG);
            /*  Get the target nodes number. */
            int targetNodeNumber = get(boost::vertex_index1, *backwardDAG, targetNode);
            /*  Retrieve the variables for the target. */
            instructionVariables targetVars = variableMap.find(targetNodeNumber)->second;
            /*  Check if the maximum delay should be reassigned. */
            if (targetVars.maximumDelayToLeaf < (visitedVars.maximumDelayToLeaf + visitedVars.executionTime)) {
                /*  The maximum delay in the child node is less. Assign the new
                    maximum delay to the child. */
                targetVars.maximumDelayToLeaf = visitedVars.maximumDelayToLeaf + visitedVars.executionTime;
                /*  Debug printout. */
                if (debuging) {
                    /*  Get the instruction pointer. */
                    SgAsmMipsInstruction* targetMips = get(boost::vertex_name, *backwardDAG, targetNode);
                    /*  Printout. */
                    std::cout << "Maximum delay set to " << targetVars.maximumDelayToLeaf
                        << " in instruction." << std::endl;
                    printInstruction(targetMips);
                    std::cout << std::endl;
                }
                /*  Save the new variables in the map. */
                variableMap[targetNodeNumber] = targetVars;
                /*  Push the child node onto the visit queue so it will be visited. */
                visitQueue.push(targetNode);
            }
        }

    }
}

/*  Calculates the slack variable, which is the difference between EST and LST. */
void listScheduler::calculateSlack(graphDAG* blockDAG) {
    /*  Pointer to the map. */
    frameworkDAG* forward = blockDAG->getForwardDAG();
    /*  Debug print. */
    if (debuging) {
        std::cout << "Calculating slack for all nodes." << std::endl;
    }
    /*  Iterate through the variable map and update values. */
    for(nodeVars::iterator varMapIter = variableMap.begin();
        varMapIter != variableMap.end(); ++varMapIter) {
        /*  Get the variable struct. */
        instructionVariables currentVars = varMapIter->second;
        /*  Calculate the slack value. */
        currentVars.slack = currentVars.latestStart - currentVars.earliestStart;
        /*  Debug print. */
        if (debuging) {
            /*  Get instruction pointer. */
            DAGVertexDescriptor node = nodeNumberToVertex.find(varMapIter->first)->second;
            SgAsmMipsInstruction* mips = get(boost::vertex_name, *forward, node);
            std::cout << "Slack " << currentVars.slack << " = " << currentVars.latestStart << " - " << currentVars.earliestStart
                << " for instruction. " << std::endl;
            printInstruction(mips); 
            std::cout << std::endl;
        }
        /*  Save the new variables. */
        varMapIter->second = currentVars;
    }
}

