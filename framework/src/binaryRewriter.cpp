

/* Framework header */
#include "binaryRewriter.hpp"


// Constructor which takes the path to the file as input.
BinaryRewriter::BinaryRewriter(int argc, char **binaryFile) {
    //build ast and cfg.
    initialize(argc, binaryFile);
}

//destructor
//BinaryRewriter::~BinaryRewriter() {

//}

// Pass the binary to the frontend.
void BinaryRewriter::initialize(int argc, char **binaryFile) {
    
    //variables.
    decisionsMade = 0;

    // Create a cfg to be used. 
    CfgPtr = new Cfg;

    //initialize the instruction struct.
    instInfo.kind = mips_unknown_instruction;
    instInfo.mnemonic = "";
    instInfo.instructionConstant = 0;

    // Call frontend to parse the file, save it in the private variable.
    binaryProjectPtr = frontend(argc, binaryFile);
    // Extract the SgAsmInterpretation to use when building the cfg
    std::vector<SgAsmInterpretation*> asmInterpretations = SageInterface::querySubTree<SgAsmInterpretation>(binaryProjectPtr);    
    // Set the register dictionary.
    mipsRegisters = RegisterDictionary::dictionary_for_isa(asmInterpretations.back());
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
                //Need to deconstruct the current instruction before calling the decision function.
                deconstructInstruction();
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

//function returns the instruction struct.
instructionInformation BinaryRewriter::getInstructionInfo() {
    return instInfo;
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
    //!!!! The added instruction should now be deconstructed.
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
//deconstructs the current function. Making information about available.
//The inspected instruction pointer is set to the latest instruction here.
instructionInformation BinaryRewriter::deconstructInstruction() {
    SgAsmMipsInstruction* instruction = inspectedInstruction;
    //
    instructionInformation instructionStruct;
    //set instruction kind
    instructionStruct.kind = instruction->get_kind();
    //set mnemonic
    instructionStruct.mnemonic = instruction->get_mnemonic(); 
    //save the operands list.
    instructionStruct.operandsList = instruction->get_operandList();    
    //clear the registers vector.
    instructionStruct.registers.clear();
    //save the operand expressions.
    //instructionStruct.operandExpressionList = &instructionStruct.operandsList->get_operands();
    //decode the instruction to get the registers and constants
    //decodeOperands();
    //
    return instructionStruct;
}

//deconstructs the operands of the instructions to registers or constants.
void BinaryRewriter::decodeOperands(){
    //go through the operands and identify it and save the information.
    //get the expression list for the operands.
    SgAsmExpressionPtrList* operandExpressionList = &instInfo.operandsList->get_operands(); 
    //go through the expressions and identify them.
    for (int i = 0; i < operandExpressionList->size(); i++) {
        decodeExpression((*operandExpressionList)[i]);
    }

}

//decodes one operand of an instruction.
//operandnumber is probably needed to keep track of which operand we are decoding.
//This is because i need to know if a operand is input or output.
//combine with checking what instruction it is i might need to 
void BinaryRewriter::decodeExpression(SgAsmExpression* operandExpr) {
    //Check what kind of expression it is, loking at the variantT.
    switch(operandExpr->variantT()) {
    case V_SgAsmDirectRegisterExpression: {
        //Register expression. 
        //Cast the expression to the right type.
        SgAsmDirectRegisterExpression* regExpr = isSgAsmDirectRegisterExpression(operandExpr);
        //get the registerdescriptor and save it
        RegisterDescriptor reg = regExpr->get_descriptor();
        //get the register string name.
        std::string regName = mipsRegisters->lookup(reg);
        //Insert into the vector
        instInfo.registers.push_back(std::pair<std::string, RegisterDescriptor>(regName, reg));
        break;
        }
    case V_SgAsmMemoryReferenceExpression: {
        //Memory expression, contains another expression for the constant and register.
        SgAsmMemoryReferenceExpression* memRef = isSgAsmMemoryReferenceExpression(operandExpr);
        //decode the size/number of bits for the reference.
        SgAsmType* refType = memRef->get_type();
        //save the bits/size.
        instInfo.memoryReferenceSize = refType->get_nBits();
        //continue with extractin the address.
        decodeExpression(memRef->get_address());
        break;
    }
    case V_SgAsmBinaryAdd: {
        //Addition of two expressions, e.g. fp + constant.
        SgAsmBinaryExpression* binaryAdd = isSgAsmBinaryExpression(operandExpr);
        //decode rhs
        decodeExpression(binaryAdd->get_rhs());
        //decode lhs
        decodeExpression(binaryAdd->get_lhs());
        break;
    }
    case V_SgAsmIntegerValueExpression: {
        //Expression for constants, e.g. constants in lw and sw instructions.
        //do i need to save the significant bits?
        //get the value and save it.
        SgAsmIntegerValueExpression* valueExpr = isSgAsmIntegerValueExpression(operandExpr);
        //save the constant
        instInfo.instructionConstant = valueExpr->get_absoluteValue();
        //save the significant bits, do i need to?
        instInfo.significantBits = valueExpr->get_significantBits();
        break;
    }
    default: {
        //This case should not be reached.
    }
    }
    
}

/******************************************************************************
* Configuration functions.
******************************************************************************/

//Select method allocation method
void BinaryRewriter::selectRegisterAllocation() {

}
//Select scheduling method.
void BinaryRewriter::selectInstructionScheduling() {

}
