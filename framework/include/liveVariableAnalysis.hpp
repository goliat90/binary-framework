/*  Live-variable analysis.
*   Given a control flow graph the definition and use of variables
*   are computed for each basic block. Then live-variable analysis is applied
*   to the control flow graph, lastly the live intervals are computed.
*/

/*  Includes    */
#include "cfgHandler.hpp"


class liveVariableAnalysis {
    public:
    liveVariableAnalysis(CFG*);

    private:
    /*  Hide default constructor    */
    liveVariableAnalysis();

};

