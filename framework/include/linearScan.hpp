/* Header file for Linear Scan algorithm */
#ifndef LINEAR_SCAN_H
#define LINEAR_SCAN_H

/*  Includes    */
#include "liveVariableAnalysis.hpp"
#include "cfgHandler.hpp"

class linearScanHandler {
    public:
    /*  Constructor */
    linearScanHandler(CFGhandler*);
    /*  Call to apply linear scan allocation */
    void applyLinearScan();
    /*  Select debug mode */
    void selectDebuging(bool);
    private:
    /*  Hide default constructor */
    linearScanHandler();
    /*  Variables */
    /*  debug mode. */
    bool debuging;
    /*  Cfg handler */
    CFGhandler* cfgHandlerPtr;
    /*  live-range analysis handler */
    liveVariableAnalysisHandler* liveRangeHandler;
    /*  Container for free available registers to allocate */
    std::set<mipsRegisterName> freeRegisters;
    
    /*  Linear scan allocation function */
    void linearScanAllocation();
    /*  Expire old live interval */
    //TODO need to figure out the argument
    void expireOldInterval();
    /*  Spill at interval */
    //TODO need to figure out the argument
    void spillAtInterval();
    /*  Replace physical registers with symbolics before analysis and allocation */
    void replaceHardRegisters();
};

#endif

