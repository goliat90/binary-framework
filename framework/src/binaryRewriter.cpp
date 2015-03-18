

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
    CfgPtr = new Cfg;

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
    basicBlockMap bbMap = get(boost::vertex_name, *CfgPtr);
    
    for(std::pair<CfgVIter, CfgVIter> VPair = vertices(*CfgPtr);
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
                //For each instruction check what should be done.
                transformDecision(*stmtIter);
            }
        }        
        //the block has been traversed. Swap the shadowBlock with the original basic block.
        originalStatementListPtr->swap(*shadowStatementListPtr);
    }
}

/* Functions related to instruction manipulation */

//The desicion function that users can overwrite.
//Here it will just be an empty function.
void BinaryRewriter::transformDecision(SgAsmStatement* instPtr) {
    //std::cout << "Framework decision function" << std::endl;
    decisionsMade++;
    //printout of the instruction and the number of operands.
    std::cout << getInstructionMnemonic() << " " << getInstructionOperands().size() << std::endl;
    saveInstruction();
}

//return what kind of mips enum instruction it is.
MipsInstructionKind BinaryRewriter::getInstructionKind() {
    return inspectedInstruction->get_kind(); 
}

//return the operands list for the inspected instruction.
SgAsmExpressionPtrList BinaryRewriter::getInstructionOperands() {
    return inspectedInstruction->get_operandList()->get_operands();
}

//return the mips instruction mnemonic string.
std::string BinaryRewriter::getInstructionMnemonic() {
    return inspectedInstruction->get_mnemonic();
}

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


//Configuration functions.

//Select method allocation method
void BinaryRewriter::selectRegisterAllocation() {

}
//Select scheduling method.
void BinaryRewriter::selectInstructionScheduling() {

}
