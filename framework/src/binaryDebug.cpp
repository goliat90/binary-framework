/* Functions for debugging the framework  */

/* header file */
#include "binaryDebug.hpp"



/* print the instructions contained in a basicblock */
void printBasicBlockInstructions(SgAsmBlock* block) {
    //get the list of instructions in the block.
    SgAsmStatementPtrList* stmtlistPtr = block->get_statementList();
    //iterate through the statement list and print.
    for(SgAsmStatementPtrList::iterator instIter = stmtlistPtr->begin();
        instIter != stmtlistPtr.end(); instIter++) {
        //decode the instruction.
        
    }
}

