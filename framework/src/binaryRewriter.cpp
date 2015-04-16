
/* Framework header */
#include "binaryRewriter.hpp"


// Constructor which takes the path to the file as input.
BinaryRewriter::BinaryRewriter(int argc, char **binaryFile) {
    //build ast and cfg.
    initialize(argc, binaryFile);
}


// Pass the binary to the frontend.
void BinaryRewriter::initialize(int argc, char **binaryFile) {
    
    //variables.
    decisionsMade = 0;

    // Create a cfg to be used. 
    CfgPtr = new CFG;

    // Call frontend to parse the file, save it in the private variable.
    binaryProjectPtr = frontend(argc, binaryFile);
    // Extract the SgAsmInterpretation to use when building the cfg
    std::vector<SgAsmInterpretation*> asmInterpretations = SageInterface::querySubTree<SgAsmInterpretation>(binaryProjectPtr);    
    // build a cfg as well.
    rose::BinaryAnalysis::ControlFlow cfgAnalyzer;
    cfgAnalyzer.build_block_cfg_from_ast(asmInterpretations.back(), *CfgPtr);
}

void BinaryRewriter::printInformation() {
    std::cout << "Decisions made " << decisionsMade << std::endl;
}


// Does the actual traversal and applies transformations to the binary.
//This function will traverse the block cfg.
void BinaryRewriter::traverseBinary() {
    //get the map for the basic blocks.
    basicBlockPropertyMap bbMap = get(boost::vertex_name, *CfgPtr);
    
    for(std::pair<CFGVIter, CFGVIter> VPair = vertices(*CfgPtr);
        VPair.first != VPair.second; ++VPair.first) {
        //Retrieve the basic block from the map by using the vertex as key.
        SgAsmBlock* currentBlock = bbMap[*VPair.first];
        //Get the original statement list to a pointer.
        SgAsmStatementPtrList* originalStatementListPtr = &currentBlock->get_statementList(); 
        //Initialize the shadowStatementList.
        shadowStatementListPtr = new SgAsmStatementPtrList; 
        //extract the statement list from the basic block, statement list is a SgAsmStatement* vector.
        for(SgAsmStatementPtrList::iterator stmtIter = originalStatementListPtr->begin();
            stmtIter != originalStatementListPtr->end(); stmtIter++) {
            //Perhaps check if the statement is actually an instruction?
            //this if is currently not needed i think, just a test.
            if ((*stmtIter)->variantT() == V_SgAsmMipsInstruction) {
                //Set the currently inspected instruction for the framework
                //inspectedInstruction = static_cast<SgAsmMipsInstruction*>(*stmtIter);
                inspectedInstruction = isSgAsmMipsInstruction(*stmtIter);
                //Need to deconstruct the current instruction before calling the decision function.
                //deconstructInstruction();
                //For each instruction check what should be done.
                transformDecision(*stmtIter);
            }
        }        
        //the block has been traversed. Swap the shadowBlock with the original basic block.
        originalStatementListPtr->swap(*shadowStatementListPtr);
    }
}


/******************************************************************************
* Functions providing different information of the current instruction.
******************************************************************************/

//The desicion function that users can overwrite.
//Here it will just be an empty function.
void BinaryRewriter::transformDecision(SgAsmStatement* instPtr) {
    //std::cout << "Framework decision function" << std::endl;
    decisionsMade++;
    //printout of the instruction and the number of operands.
    saveInstruction();
}

/******************************************************************************
* Insert, delete and save instructions
******************************************************************************/
//Inserts an instruction into the shadow statementlist.
//This could be the original instruction or one provided by
//a user defined descision function.
void BinaryRewriter::insertInstruction(SgAsmStatement* addedInstruction) {
    //The passed instruction from the user, inserted into the shadow list.
    shadowStatementListPtr->push_back(addedInstruction);
    //!!!! The added instruction should now be deconstructed or should it?
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
