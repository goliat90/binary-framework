/* Functions for debugging the framework  */

/* header file */
#include "binaryDebug.hpp"


/* print the instructions contained in a basicblock */
void printBasicBlockInstructions(SgAsmBlock* block) {
    //get the list of instructions in the block.
    SgAsmStatementPtrList* stmtlistPtr = &block->get_statementList();
    /* print the block number */

    //iterate through the statement list and print.
    for(SgAsmStatementPtrList::iterator instIter = stmtlistPtr->begin();
        instIter != stmtlistPtr->end(); instIter++) {
        /* Cast the SgAsmStatement to SgAsmMipsInstruction */
        SgAsmMipsInstruction* mipsInst = isSgAsmMipsInstruction(*instIter);
        /* decode the instruction and print the information */
        instructionStruct instruction;
        //i cant call the function for some reason
        //instruction = decodeInstruction(mipsInst);
        /* Print the address, nemonic, registers, constant */

    }
    /* print some delimiter as well */
}

/* Prints out relevant information about a instruction */


/* Print input registers */

/* Print output registers */

/* Print constant? might be redundant */


