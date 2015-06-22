#ifndef LIVEVARIABLE_H
#define LIVEVARIABLE_H
/*  Live-variable analysis.
*   Given a control flow graph the definition and use of variables
*   are computed for each basic block. Then live-variable analysis is applied
*   to the control flow graph, lastly the live intervals are computed.
*/

/*  Includes    */
#include "cfgHandler.hpp"
#include "mipsISA.hpp"

/*  Boost includes  */
//TODO consider using boost dynamic bitset for variable representation
//TODO this would make it possible to use bit operators for computations.

/*  Typedefs    */
typedef std::pair<std::set<registerStruct>, std::set<registerStruct> > inoutPair;

class liveVariableAnalysisHandler {
    public:
    /*  Constructor */
    liveVariableAnalysisHandler(CFG*);
    /*  Returns some kind of structure with live intervals
        or is a query function perhaps  */
    //TODO add function head.

    private:
    /*  Hide default constructor    */
    liveVariableAnalysisHandler();
    /*  Definition and use function */
    void computeDefAndUse();
    /*  Live variable analysis function */
    void computeLiveAnalysis();
    /*  Live interval function, finds the intervals,
        depth first search traversal.   */
    void findLiveIntervals();

    /*  Variables   */
    CFG* functioncfg;
    /*  Storage for def and use of basic blocks. */
    std::map<rose_addr_t, inoutPair*> inoutMap;

    /*  Storage for IN and OUT of basic blocks. */

    /*  Map to track visited blocks when determining depth first search
        order.  */

    /*  Storage representation for live intervals.  */

};

#endif 

