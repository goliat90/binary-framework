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
#include "boost/dynamic_bitset.hpp"
//TODO this would make it possible to use bit operators for computations.

/*  Typedefs    */
/*  pair containing the definition and usage of variables in a block, order
    is first = definition, second = usage. */
typedef std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<> > defuseBits;

class liveVariableAnalysisHandler {
    public:
    /*  Constructor */
    liveVariableAnalysisHandler(CFG*);
    /*  Returns some kind of structure with live intervals
        or is a query function perhaps  */
    //TODO add function head.
    /*   enable debuging    */
    void setDebug(bool);

    private:
    /*  Hide default constructor    */
    liveVariableAnalysisHandler();
    /*  Definition and use function */
    void computeDefAndUse();
    /*  Checks source registers for use */
    void instructionUsageAndDefinition(instructionStruct*, defuseBits*);
    /*  Count the symbolic registers present */
    int countSymbolicRegisters();
    /*  Live variable analysis function */
    void computeLiveAnalysis();
    /*  Live interval function, finds the intervals,
        depth first search traversal.   */
    void findLiveIntervals();

    /*  Variables   */
    /*  Switch debuging on and off. */
    bool debuging;
    /*  Pointer to function cfg */
    CFG* functioncfg;

    /*  Storage for def and use of basic blocks, block address is used as key. */
    std::map<SgAsmBlock*, defuseBits> defuseBlockMap;
    /*  map between the symbolic register and their bit */
    std::map<unsigned, int> symbolicToBit;

    /*  map to which symbolic register is represented by what bit */
    //boost::dynamic_bitset<> 

    /*  Storage for IN and OUT of basic blocks. */

    /*  Storage representation for live intervals.  */

    /*  Map to track visited blocks when determining depth first search
        order.  */

};

#endif 

