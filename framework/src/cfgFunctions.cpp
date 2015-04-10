/* CFG functions */

#include "cfgFunctions.hpp"


/* Makes a cfg for a specific function */
CFG* createFunctionCFG(CFG* programCFG, std::string functionName) {
    /* New cfg variable */
    CFG* functionCFG = new CFG;
    /* track visited blocks */
    std::map<SgAsmBlock *, bool> visitedBlock;
    /* track mapping between programcfg and functioncfg vertices */
    std::map<CFG::vertex_descriptor, CFG::vertex_descriptor> vertexMap;
    /* keep track of which blocks are included in the functioncfg */
    std::map<CFG::vertex_descriptor, bool> functionVertex;

    

    return functionCFG;
}



