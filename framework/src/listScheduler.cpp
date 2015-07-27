
/*  Includes */
#include "listScheduler.hpp"


/*  Constructor to be used. */
listScheduler::listScheduler(CFGhandler* passedHandler) {
    /*  Debug set as false. */
    debuging = false;
    /*  Saved the passed pointer. */
    cfgObject = passedHandler;

    //TODO call the graph dag here?
    //graphDAG DAGobject(cfgObject->getProgramCFG());
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
    /*  Build dag object. */
    graphDAG blockDAG(basic);
    /*  Enable debuging in DAG code if set. */
    if (debuging) {
        blockDAG.setDebuging(debuging);
    }
    /*  Let it build DAGs. */
    blockDAG.buildDAGs();

    //TODO create and calculate the variables used for scheduling.
    /*  Visitor object. */
    listBFSVisitor listVariableBuilder;
    /*  Get forward DAG. */
    frameworkDAG* forwardDAG = blockDAG.getForwardDAG();
    /*  In the forward pass the first instruction is the root. */
    DAGVertexDescriptor* forwardRoot = blockDAG.getFirstInstructionVertex();
    /*  Perform a forward pass in the DAG. */
    if (debuging) {
        std::cout << "Performing forward pass on DAG." << std::endl;
    }
    breadth_first_search(*forwardDAG, *forwardRoot,
        boost::visitor(listVariableBuilder));

    /*  Perform a backward pass in the DAG. */
    /*  Get the backward DAG. */
    frameworkDAG* backwardDAG = blockDAG.getBackwardDAG();
    /*  In the backward pass the last instruction is the root. */
    DAGVertexDescriptor* backwardRoot = blockDAG.getLastInstructionVertex();
    /*  Perform the backward pass. */
    if (debuging) {
        std::cout << "Performing backward pass on DAG." << std::endl;
    }
//    breadth_first_search(*backwardDAG, *backwardRoot,
//        boost::visitor(listVariableBuilder));

    //TODO do the actuall scheduling.
    
}


