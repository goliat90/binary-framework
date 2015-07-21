
/*  Includes */
#include "listScheduler.hpp"


/*  Constructor to be used. */
listScheduler::listScheduler(CFGhandler* passedHandler) {
    /*  Saved the passed pointer. */
    cfgObject = passedHandler;

    //TODO call the graph dag here?
    //graphDAG DAGobject(cfgObject->getProgramCFG());
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
        /*  Call block scheduling function. */
        scheduleBlock(blockToSchedule);
    }
}



/*  Handles scheduling a block according to scheduling variables.
    A DAG is built for the block and with it it is scheduled. */
void listScheduler::scheduleBlock(SgAsmBlock* basic) {
    /*  Build dag object. */
    graphDAG blockDAG(basic);
    /*  Let it build DAGs. */
    blockDAG.buildDAGs();
    
}
