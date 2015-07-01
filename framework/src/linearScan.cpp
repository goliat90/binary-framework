/* Source code for the framework linear scan */



/* includes */
#include "linearScan.hpp"


/*  Constructor */
linearScanHandler::linearScanHandler(CFGhandler* passedCfgObject){
    /*  Set Default values */
    debuging = false;
    /*  Save the pointer to the cfg handler */
    cfgHandlerPtr = passedCfgObject;
    /*  Create the live variable analysis object */
    liveRangeHandler = new liveVariableAnalysisHandler(cfgHandlerPtr->getFunctionCFG());
}


/*  Activate or deactivate debuging printout*/
void linearScanHandler::selectDebuging(bool mode) {
    debuging = mode;
    /*  pass it to the liverange analysis */
    liveRangeHandler->setDebug(mode);
    
}

/*  Applies the linear scan */
void linearScanHandler::applyLinearScan() {
    //TODO Might have to add here some kind of funciton to
    //TODO free up the physical registers before live range analysis

    /*  Get live-range analysis done before performing register allocation. */
    liveRangeHandler->performLiveRangeAnalysis();

}


/*  Algorithms for linear scan. */ 
void linearScanHandler::linearScanAllocation() {
/*
Active is a list of active live intervals
active list <- empty
foreach live interval i, in order of increasing start point
    EXPIREOLDINTERVAL(i)
    if length(active) = R then
        SPILLATINTERVAL(i)
    else
        register[i] <- a register removed from pool of free registers
        add i to active, sorted by inreasing end point
*/

}


void linearScanHandler::expireOldInterval() {
/*
foreach interval j in active, in order of increasing end point
    if endpoint[i] >= startpoint[i] then
        return
    remove j from active
    add register[j] to pool of free registers
*/
}


void linearScanHandler::spillAtInterval() {
/* 
spill <- last interval in active
if endpoint[spill] > endpoint[i] then
    register[i] <- register[spill]
    location[spill] <- new stack location
    remove spill from active
    add i to active, sorted by increasing end point
else
    location[i] <- new stack location
*/

}
