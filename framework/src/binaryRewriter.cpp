
/* Framework header */
#include "binaryRewriter.hpp"


// Constructor which takes the path to the file as input.
BinaryRewriter::BinaryRewriter(int argc, char **binaryFile) {
    /* build ast and cfg. */
    initialize(argc, binaryFile);
    /* Set default values on some of the variables  */
    debugging = false;
    decisionsMade = 0;
}


// Pass the binary to the frontend.
void BinaryRewriter::initialize(int argc, char **binaryFile) {
    
    //variables.
    decisionsMade = 0;

    // Call frontend to parse the file, save it in the private variable.
    binaryProjectPtr = frontend(argc, binaryFile);

    /* initialize the cfghandler */
    cfgContainer = new CFGhandler;
    /* pass the project to build the programcfg */
    cfgContainer->initialize(binaryProjectPtr);
}

/* Used to select the function to be transformed and builds the functioncfg */
void BinaryRewriter::functionSelect(std::string fName) {
    /* call on cfghandler to build the functioncfg */
    cfgContainer->createFunctionCFG(fName);
}

void BinaryRewriter::printInformation() {
    std::cout << "Decisions made " << decisionsMade << std::endl;
}


// Does the actual traversal and applies transformations to the binary.
//This function will traverse the block cfg.
void BinaryRewriter::transformBinary() {

    /* Traverse the function cfg and apply the user transformations.
        Get the function CFG and traverse its blocks. */
    CFG* functionGraph = cfgContainer->getFunctionCFG();


    /* Iterater through all the blocks and apply transformations */
    for(std::pair<CFGVIter, CFGVIter> vPair = vertices(*functionGraph);
        vPair.first != vPair.second; ++vPair.first) {
        /* get the basic block from the property map */
        SgAsmBlock* currentBB = get(boost::vertex_name, *functionGraph, *vPair.first);
        /* If debugging is active then print the block before transformation */
        if (debugging) {
            std::cout << std::endl;
            printBasicBlockInstructions(currentBB);
        }
        /* get the statement list of the block, which is the instructions */
        SgAsmStatementPtrList* orgStmtPtrList = &currentBB->get_statementList();
        /* Initialize the shadowstatement list */
        shadowStatementListPtr = new SgAsmStatementPtrList;
        /* Iterate through the statment list and check each instruction */
        for(SgAsmStatementPtrList::iterator stmtIter = orgStmtPtrList->begin();
            stmtIter != orgStmtPtrList->end(); ++stmtIter) {
            /* Check that the statement is a mipsinstruction and if it is
                forbidden or not. */
            if ((*stmtIter)->variantT() == V_SgAsmMipsInstruction) {
                /* cast the instruction to mips */
                inspectedInstruction = isSgAsmMipsInstruction(*stmtIter);
                /* check if the instruction is allowed to be transformed or not */ 
                if(cfgContainer->isForbiddenInstruction(inspectedInstruction) == false) {
                    /* The instruction is allowed to be transformed.
                        Call the user decision function. */
                    transformDecision(inspectedInstruction);
                }
            }
        }
        /* The blocks statement list has been traversed. swap the list with
            the shadow list and continue with the next block */
        orgStmtPtrList->swap(*shadowStatementListPtr);
        if (debugging) {
            std::cout << "Block transformed" << std::endl;
            printBasicBlockInstructions(currentBB);
        }
    }

    /* Debug print, print all the blocks and their instructions */
    
    /* Apply naive or optimized transformation */

    /* Debug print */

    /* Correct addresses */

    /* Debug print */

    /* Correct branch instructions */

    /* Debug print */

    /* Adjust segment sizes */

}


/******************************************************************************
* Functions providing different information of the current instruction.
******************************************************************************/

//The desicion function that users can overwrite.
//Here it will just be an empty function.
void BinaryRewriter::transformDecision(SgAsmMipsInstruction* instPtr) {
    //std::cout << "Framework decision function" << std::endl;
    decisionsMade++;
    //printout of the instruction and the number of operands.
    saveInstruction();
}

/******************************************************************************
* Insert, delete, save and move instructions
******************************************************************************/
//Inserts an instruction into the shadow statementlist.
//This could be the original instruction or one provided by
//a user defined descision function.
void BinaryRewriter::insertInstruction(SgAsmStatement* addedInstruction) {
    //The passed instruction from the user, inserted into the shadow list.
    shadowStatementListPtr->push_back(addedInstruction);
}

//Removes an instruction during the transformation. Basically it will just
//skip copying the instruction over to the shadow block. It does nothing
//perhaps remove it fully?
void BinaryRewriter::removeInstruction() {
}

//Used when the original instruction is to be preserved.
//This function will copy over the current instruction to the shadow statementlist.
void BinaryRewriter::saveInstruction() {
    //insert the instruction at the end of the statement list.
    shadowStatementListPtr->push_back(inspectedInstruction);
}


/******************************************************************************
* Private functions for the framework
******************************************************************************/


/******************************************************************************
* Configuration functions.
******************************************************************************/

//Select method allocation method
void BinaryRewriter::selectRegisterAllocation() {

}
//Select scheduling method.
void BinaryRewriter::selectInstructionScheduling() {

}

/* enable disable debugging */
void BinaryRewriter::setDebug(bool setting) {
    debugging = setting;
}
