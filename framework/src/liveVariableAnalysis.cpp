/*  Live-variable analysis.
*   Given a control flow graph the definition and use of variables
*   are computed for each basic block. Then live-variable analysis is applied
*   to the control flow graph, lastly the live intervals are computed.
*/

#include "liveVariableAnalysis.hpp"

/*  Constructor */
liveVariableAnalysisHandler::liveVariableAnalysisHandler(CFG* subroutine) {
    /* Save the cfg */
    functioncfg = subroutine;
}


/*  Function computes the def and use for each basic block. Iterates through
    each basic block and finds the def and use for the block.   */
void liveVariableAnalysisHandler::computeDefAndUse() {
    /*  Variables   */
    

    /*  Iterate through the vertices and all the basic blocks, inspect each
        instruction for def and use of symbolic registers. Save the defs
        and uses for the block. */
    for(std::pair<CFGVIter, CFGVIter> iterPair = vertices(*functioncfg);
        iterPair.first != iterPair.second; ++iterPair.first) {
        /*  Extract the basic block for the vertex then its statementlist  */
        SgAsmBlock* basic = get(boost::vertex_name, *functioncfg, *iterPair.first);
        SgAsmStatementPtrList& blockStmtList = basic->get_statementList();
        /*  Iterate through the statements and check the registers used */
        for(SgAsmStatementPtrList::iterator stmtIter = blockStmtList.begin();
            stmtIter != blockStmtList.end(); ++stmtIter) {
            /*  Decode the instructions and check the registers for symbolic ones.  */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /* Cast pointer and decode. */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                instructionStruct decodedMips = decodeInstruction(mips);
                /* Check source registers */
                //TODO consider making a function here
                /* Check destination registers */
                //TODO consider making a function here
            }
        }
    }
    /* All blocks have had their def and use computed. */
}




/*  Function performs live-analysis */
void liveVariableAnalysisHandler::computeLiveAnalysis() {

}


/*  Uses the live-variable analysis to find the live intervals  */
void liveVariableAnalysisHandler::findLiveIntervals() {

}

