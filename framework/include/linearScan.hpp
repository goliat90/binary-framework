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
    /*  debug mode. */
    bool debuging;
    /*  Cfg handler */
    CFGhandler* cfgHandlerPtr;
    /*  live-range analysis handler */
    liveVariableAnalysisHandler* liveRangeHandler;
    /*  Variables used in the linear scan register allocation.
        Contains the end point for the interval and its name. */
    intervalMap activeMap;
    /*  Container for free available registers to allocate.
        It is used as a queue in the allocation but i use list container
        so it can be searchable. */
    std::list<mipsRegisterName> registerPool;
    /*  Pointers to the start and end point boost bimaps. */
    intervalMap* startPointMap;
    intervalMap* endPointMap;
    /*  Map storing which interval has been given a register. */
    std::map<unsigned, mipsRegisterName> allocationMap;
    /*  Map to intervals spilled and their stack location. */
    std::map<unsigned, uint64_t> spillMap;
    /*  Stack offset counter. */
    uint64_t stackOffset;
    /*  Spill register. */
    mipsRegisterName spillReg;

    /*  Functions. */
    /*  Replace temporary registers with symbolic names before live-range analysis.
        The registers used by linear scan is t0-7, t8,t9. */
    void replaceHardRegisters();
    /*  Linear scan allocation function */
    void linearScanAllocation();
    /*  Expire old live interval */
    void expireOldIntervals(int);
    /*  Spill at interval */
    void spillAtInterval(intervalMap::left_iterator);
    /*  Modifies stack appropriateley. */
    void linearStackModification();
    /*  Go through the instructions and replace the symbolic registers. */
    void replaceSymbolicRegisters();
};

#endif

