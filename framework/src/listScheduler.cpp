
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
        /*  Print block if debuging. */
        if (debuging) {
            std::cout << "Scheduling block." << std::endl;
            printBasicBlockInstructions(blockToSchedule);
        }
        /*  Call block scheduling function. */
        scheduleBlock(blockToSchedule);
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

    //TODO do the actual scheduling.
    
}


