/* Header file for Linear Scan algorithm */
#ifndef LINEAR_SCAN_H
#define LINEAR_SCAN_H

/*  Includes    */
#include "liveVariableAnalysis.hpp"
#include "mipsISA.hpp"
#include "symbolicRegisters.hpp"
#include "cfgHandler.hpp"
#include "binaryDebug.hpp"

/*  Class for the linear scan algorithm. It does not use
    all registers available, it uses t0-t7, t8,t9. To use them
    it will replace all instances of these registers with symbolic
    names before performing live-variable analysis.
    A register will be used for spill. */
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
    /*  Container mapping a physical register to a symbolic registers */
    std::map<unsigned, mipsRegisterName> registerAllocationMap;

    /*  Replace temporary registers with symbolic names before live-range analysis.
        The registers used by linear scan is t0-7, t8,t9. */
    void replaceHardRegisters();
    /*  Linear scan allocation function */
    void linearScanAllocation();
    /*  Expire old live interval */
    //TODO need to figure out the argument
    void expireOldInterval();
    /*  Spill at interval */
    //TODO need to figure out the argument
    void spillAtInterval();
};

#endif

