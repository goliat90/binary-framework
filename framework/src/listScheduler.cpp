
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
    if (debuging) {
        std::cout << "----------------------------------------" << std::endl;
    }
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
    calculateSlack();


    if (debuging) {
        std::cout << "----------------------------------------" << std::endl
                << "Variable calculations complete. Scheduling block." << std::endl;
    }
    //TODO do the actuall scheduling.
    forwardListScheduling(&blockDAG);
}


/*  Comparator used in the readylist. It will enfore strict weak ordering,
    that means that true should be returned if instA should be before instB.
    It will order instructions according to the priority of the variables.
    If there is an equal value in one variable the next variable is checked.

    Order of variables checked are.
    1. Critical path, instruction on critcal path have highest priority.
    2. EST, schedule instruction as early as possible.
    3. Slack, small slack means that the window between EST and LST is little.
    4. Maximum delay to leaf, the maximum possible time for this instruction
       path to reach the leaf node.
    5. Execution time. */
bool priorityCompare(const instructionVariables& instA, const instructionVariables& instB) {
    /*  Check if there is any of the instructions are on the critical path. */
    if (0 == instA.slack && 0 == instB.slack) {
        /*  Both instructions are on critical path, check next variable. */
        if (instA.earliestStart == instB.earliestStart) {
           /*   Equal EST values, check next variable. */ 
           if (instA.slack == instB.slack) {
                /*  Equal slack, check next variable. */
                if (instA.maximumDelayToLeaf == instB.maximumDelayToLeaf) {
                    /*  Equal maximum delay to leaf, check next variable.
                        Comparing execution times, higher execution times are prioritized. */
                    return instA.executionTime > instB.executionTime;

                } else if (instA.maximumDelayToLeaf > instB.maximumDelayToLeaf) {
                    /*  instA has higher delay and therefore higher priority. */
                    return true;
                } else {
                    /*  instB has the higher priority. */
                    return false;
                }
           } else if (instA.slack < instB.slack) {
                /*  instA has a smaller slack and has higher priority. */
                return true;
           } else {
                /*  instB has smaller slack, therefore higher priority. */
                return false;
           }
        } else if (instA.earliestStart < instB.earliestStart) {
            /*  instA has a lower EST than instB and har higher priority. */
            return true;
        } else {
            /*  If none of the above cases match then it means that instB
                has lower EST than instA. */
            return false;
        }
    } else if (0 == instA.slack) {
        /*  InstA is on critical path and should be before instB which is
            not on critical path. */
        return true;
    } else {
        /*  InstB is on critical path and should be before instA which is
            not on critical path. */
        return false;
    }
}

/*  Perform list scheduling. */
void listScheduler::forwardListScheduling(graphDAG* DAGobject) {

    /*  Variable declarations. */
    int listCycle = 0;
    /*  Instructions ready for scheduling, ordered by priority. */
    std::list<instructionVariables> readyList;
    std::list<instructionVariables> inFlightList;
    //TODO  Maybe i need a list containing scheduling times for instructions.
    //TODO  When the instruction is scheduled and when finished.

    /* Initialize the readylist with the root vertex variables. */
    DAGVertexDescriptor* root = DAGobject->getForwardDAGRoot();
    readyList.push_front(variableMap.find(*root)->second);

    //TODO verify if i can list the priority with the use of comparator and .sort
    //typedef std::list<DAGVertexDescriptor*> vertexPtrList;
    //vertexPtrList readyList;
    /*  Instructions in flight. */
    //TODO Since only one instruction can be scheduled at a time this might be redundant having that list. */
    //vertexPtrList inFlightList;

    /*  The Determined list schedule. This list will afterwards be exchanged
        with the list in the block. */
    //TODO can search this list with std::find.
    SgAsmStatementPtrList scheduleOrder;

    /*  Insert root nodes into the ready list, since there is only one root node
        use the function to get the root node. */
    //readyList.push_front(DAGobject->getForwardDAGRoot());

    /*  While loops. */
    while (readyList.empty() && inFlightList.empty()) {
        /*  Sort the readyList according to priority with the priority compare. */
        readyList.sort(priorityCompare);
        /*  Find highest priority instruction that is ready and schedule it. */
        for(std::list<instructionVariables>::iterator readyIter = readyList.begin();
            readyIter != readyList.end(); ++readyIter) {
            /*  Check if the instruction can be scheduled at this cycle.
                Which means that EST should be checked. */
            //TODO verify that it is the EST that i can check. 
            //TODO Do i even need to check here? Is it if i have ready instructions
            //TODO but their dependant instruction is not finished? But why are they in the readylist?

            //TODO for now i add the first instruction to the inflight list and break the for loop.
            inFlightList.push_back(*readyIter);
            readyList.erase(readyIter);
            break;
        }

        listCycle += 1;

        /*  Check the inflight list if any instruction is finished. */

        //TODO this will probably just be a check to see if the last scheduled instruction has finished.
    }

/*
    Cycle = 0
    ready-list = root nodes in DPG (Data precedence graph), This will probably be the
    a kind of list containing nodes found in the cfg to be ready to be scheduled.
    inflight-list =  instructions that are executing. 
    //TODO the lists should probably contain nodes. Can give me the edges, source, target.
    //TODO ready-list should be sortable according to priority.

    //TODO need a function that can determine if a instructions is ready to be added to the ready-list.
    //TODO I will probably need a list of the scheduled instructions that is searchable. (std::list)

    while (ready-list or inflight-list is not empty)
        for all instructions in ready-list in descending priority order
        //TODO The for all here might not be needed since only one instruction can be scheduled at a time.
            if a functional unit exists so a instruction can be scheduled at this cycle.
                remove instruction from ready-list and add it to inflight-list.
                add instruction to the schedule at time cycle.
                if the instruction has out-going edges.(anti-edges) //TODO need to check for difference.
                    add the targets of the edges to the ready-list
                endif
            endif
        endfor

        cycle = cycle + 1
        for all instructions in the inflight-list
            if operation finished at time cycle
                remove instruction from inflight-list
                check for nodes waiting for instruction to finish and add to ready-list
                    if all operands are available.
            endif
        endfor
    endwhile.

    //TODO Do i need to annotate edges in graph? Or is some of the pseudo code not needed?
    //TODO Im thinking about the situation when nodes are added to the ready-list, do i have both cases?
*/
}






/*  Used to sort the ready instructions/nodes in the correct order accoring to priority. 
    Sorting criteria are.
    1. Critical path, instruction on critcal path have highest priority.
    2. EST, schedule instruction as early as possible.
    3. Slack, small slack means that the window between EST and LST is little.
    4. Maximum delay to leaf, the maximum possible time for this instruction
       path to reach the leaf node.
    5. Execution time. */
//void listScheduler::listPrioritySort(std::list<DAGVertexDescriptor*>& readyNodes) {
//    /*  Variables. */
//    bool elementsSwaped = true;
//    /*  Sort the list according to priority. Sort until there has been 
//        no swap in in a whole iteration of the list. */
//    while (elementsSwaped) {
//        /*  Iterate through the list and compare elements. */
//        for(std::list<DAGVertexDescriptor*>::iterator vertIter = readyNodes.begin();
//            vertIter != readyNodes.end(); ++vertIter) {
//            /*  Current element and the next element. */
//        }
//    }
//}


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
        //SgAsmMipsInstruction* nodeInst = get(boost::vertex_name, *forwardDAG, *iterPair.first);
        /*  Create an entry for the vertice instruction.
            Also set some of the variable values. */
        instructionVariables nodeVars;
        /*  Set the execution time of the instruction. */
        //TODO need to change here if i set different values on instructions. 
        nodeVars.executionTime = 1;
        /*  Save the node itself as well for later reference when scheduling. */
        nodeVars.forwardNodeRef = *iterPair.first;
        /*  Insert the variable struct into the map. */
        variableMap.insert(nodeMapPair(*iterPair.first, nodeVars));
    }
    /*  Debug print. */
    if (debuging) {
        std::cout << "Created " << variableMap.size() << " entries." << std::endl << std::endl;
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

    /*  Push the root vertex into the queue. */
    visitQueue.push(*forwardRoot);

    /*  Set the EST in the root node before traversal. */
//    SgAsmMipsInstruction* rootMips = get(boost::vertex_name, *forwardDAG, *forwardRoot);
    /*  Get the variables of the popped node. */
    instructionVariables rootVars = variableMap.find(*forwardRoot)->second;
    /*  Set the EST value in the root and save it. */
    rootVars.earliestStart = 0; 
    variableMap[*forwardRoot] = rootVars;
    /*  Debug printout. */
    if (debuging) {
        std::cout << "EST set to root node to " << rootVars.earliestStart << std::endl << std::endl;
    }

    /*  BFS algorithm. Continue as long as the queue is not empty. */
    while(!visitQueue.empty()) {
        /*  Take vertex from queue. */
        DAGVertexDescriptor visitedNode = visitQueue.top();
        /*  Remove it from the queue. */
        visitQueue.pop();
        /*  Get the instruction pointer of the popped node. */
//        SgAsmMipsInstruction* visitedMips = get(boost::vertex_name, *forwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedNode)->second;
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
            instructionVariables targetVars = variableMap.find(targetNode)->second;
            /*  Check if the EST of the target is lower than this nodes EST + Execution time. */
            if (targetVars.earliestStart < (visitedVars.earliestStart + visitedVars.executionTime)) {
                /*  The EST the visited node produces is higher than the assign so pass it over. */
                targetVars.earliestStart = visitedVars.earliestStart + visitedVars.executionTime;
                /*  Save the new EST in the mapping. */
                variableMap[targetNode] = targetVars;
                /*  Push the target node to the queue so it will be visited so it can propagate
                    its new EST. */
                visitQueue.push(targetNode);

                /*  Debug printout. */
                if (debuging) {
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

    /*  Push the root vertex into the queue. */
    visitQueue.push(*backwardRoot);

    /*  Set the correct values in the root node. */
//    SgAsmMipsInstruction* rootMips = get(boost::vertex_name, *backwardDAG, *backwardRoot);
    /*  Get the variables of the popped node. */
    instructionVariables rootVars = variableMap.find(*backwardRoot)->second;
    /*  Set the LST value in the root and save it, the LST in the root
        which is the last instruction in the block is the EST. */
    //TODO
    rootVars.latestStart = rootVars.earliestStart; 
    variableMap[*backwardRoot] = rootVars;
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
        /*  Get the instruction pointer of the popped node. */
//        SgAsmMipsInstruction* visitedMips = get(boost::vertex_name, *backwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedNode)->second;
        /*  Go through the out edges of the vertex. Check if the EST of
            the target vertice should be reassigned. */
        for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(visitedNode, *backwardDAG);
            edgePair.first != edgePair.second; ++edgePair.first) {
            /*  Get the target vertice of the edge and check if the EST should
                be reassigned. */
            DAGVertexDescriptor targetNode = target(*edgePair.first, *backwardDAG);
            /*  Get the instruction pointer. */
            SgAsmMipsInstruction* targetMips = get(boost::vertex_name, *backwardDAG, targetNode);
            /*  Retrieve the variables for the target. */
            instructionVariables targetVars = variableMap.find(targetNode)->second;
            /*  Check if the EST of the target is lower than this nodes EST + Execution time. */
            if (targetVars.latestStart > (visitedVars.latestStart - visitedVars.executionTime)) {
                /*  The EST the visited node produces is higher than the assign so pass it over. */
                targetVars.latestStart = visitedVars.latestStart - visitedVars.executionTime;
                /*  Save the new EST in the mapping. */
                variableMap[targetNode] = targetVars;
                /*  Push the target node to the queue so it will be visited so it can propagate
                    its new EST. */
                visitQueue.push(targetNode);

                /*  Debug printout. */
                if (debuging) {
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

    /*  Push the root vertex into the queue. */
    visitQueue.push(*backwardRoot);

    /*  Set the correct values in the root node. */
//    SgAsmMipsInstruction* rootMips = get(boost::vertex_name, *backwardDAG, *backwardRoot);
    /*  Get the variables of the popped node. */
    instructionVariables rootVars = variableMap.find(*backwardRoot)->second;
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
    variableMap[*backwardRoot] = rootVars;

    /*  Traverse the DAG in a BFS maner, It will revisit nodes that have their
        values updated though, a deviation from normal BFS. */
    while(!visitQueue.empty()) {
        /*  Pop node from the queue. */
        DAGVertexDescriptor visitedNode = visitQueue.top();
        /*  Remove it from the queue. */
        visitQueue.pop();
        /*  Get the instruction pointer of the popped node. */
//        SgAsmMipsInstruction* visitedMips = get(boost::vertex_name, *backwardDAG, visitedNode);
        /*  Get the variables of the popped node. */
        instructionVariables visitedVars = variableMap.find(visitedNode)->second;
        /*  Iterate over the out edges and check the maximum delay to leaf
            of the nodes children. */
        for(std::pair<DAGOEIter, DAGOEIter> edgePair = out_edges(visitedNode, *backwardDAG);
            edgePair.first != edgePair.second; ++edgePair.first) {
            /*  Get the target/child node of the currently visited node. */
            DAGVertexDescriptor targetNode = target(*edgePair.first, *backwardDAG);
            /*  Get the instruction pointer. */
            SgAsmMipsInstruction* targetMips = get(boost::vertex_name, *backwardDAG, targetNode);
            /*  Retrieve the variables for the target. */
            instructionVariables targetVars = variableMap.find(targetNode)->second;
            /*  Check if the maximum delay should be reassigned. */
            if (targetVars.maximumDelayToLeaf < (visitedVars.maximumDelayToLeaf + visitedVars.executionTime)) {
                /*  The maximum delay in the child node is less. Assign the new
                    maximum delay to the child. */
                targetVars.maximumDelayToLeaf = visitedVars.maximumDelayToLeaf + visitedVars.executionTime;
                /*  Debug printout. */
                if (debuging) {
                    std::cout << "Maximum delay set to " << targetVars.maximumDelayToLeaf
                        << " in instruction." << std::endl;
                    printInstruction(targetMips);
                    std::cout << std::endl;
                }
                /*  Save the new variables in the map. */
                variableMap[targetNode] = targetVars;
                /*  Push the child node onto the visit queue so it will be visited. */
                visitQueue.push(targetNode);
            }
        }

    }
}

/*  Calculates the slack variable, which is the difference between EST and LST. */
void listScheduler::calculateSlack() {
    /*  Debug print. */
    if (debuging) {
        std::cout << "Calculating slack for all nodes." << std::endl;
    }
    /*  Iterate through the variable map and update values. */
    for(nodeMap::iterator varMapIter = variableMap.begin();
        varMapIter != variableMap.end(); ++varMapIter) {
        /*  Get the variable struct. */
        instructionVariables currentVars = varMapIter->second;
        /*  Calculate the slack value. */
        currentVars.slack = currentVars.latestStart - currentVars.earliestStart;
        /*  Debug print. */
        if (debuging) {
            /*  Get instruction pointer. */
            std::cout << "Slack calculated to " << currentVars.slack
                << " for instruction." << std::endl;
//            printInstruction(varMapIter->first);
            std::cout << std::endl;
        }
        /*  Save the new variables. */
        varMapIter->second = currentVars;
    }
}

