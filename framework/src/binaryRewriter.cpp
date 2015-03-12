

/* Framework header */
#include "binaryRewriter.hpp"


// Constructor which takes the path to the file as input.
BinaryRewriter::BinaryRewriter(int argc, char **binaryFile) {
    //build ast and cfg.
    initialize(argc, binaryFile);
}

// Pass the binary to the frontend.
void BinaryRewriter::initialize(int argc, char **binaryFile) {
    // Create a cfg to be used. 
    blockCfgPtr = new Cfg;

    // Call frontend to parse the file, save it in the private variable.
    binaryProjectPtr = frontend(argc, binaryFile);
    // Extract the SgAsmInterpretation to use when building the cfg
    std::vector<SgAsmInterpretation*> asmInterpretations = SageInterface::querySubTree<SgAsmInterpretation>(binaryProjectPtr);    
    // build a cfg as well.
    rose::BinaryAnalysis::ControlFlow cfgAnalyzer;
    cfgAnalyzer.build_block_cfg_from_ast(asmInterpretations.back(), *blockCfgPtr);
}

void BinaryRewriter::passTransformer(userTransformer& object) {

}

void BinaryRewriter::selectRegisterAllocation() {

}

void BinaryRewriter::selectInstructionScheduling() {

}

// Does the actual traversal and applies transformations to the binary.
//This function will traverse the block cfg.
void BinaryRewriter::transformBinary() {
    //get the map for the basic blocks.
    basicBlockMap bbMap = get(boost::vertex_name, *blockCfgPtr);
    
    for(std::pair<CfgVIter, CfgVIter> VPair = vertices(*blockCfgPtr);
        VPair.first != VPair.second; ++VPair.first) {
        //Retrieve the basic block from the map by using the vertex as key.
        SgAsmBlock* currentBlock = bbMap[*VPair.first];
        //extract the statement list from the basic block.
        for(SgAsmStatementPtrList::iterator stmtIter = currentBlock->get_statementList().begin();
            stmtIter != currentBlock->get_statementList().end(); stmtIter++) {
            //For each instruction query the transformer of what to do.
            //If a transformer object has been provided then call it.
            if (transformPtr != NULL) {
                //call the function in the object here.
            }
        }     
    }
 
    //the returned instruction is swaped with the one in the map.
}



