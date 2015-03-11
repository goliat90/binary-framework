

/* Framework header */
#include "binaryRewriter.hpp"


// Constructor which takes the path to the file as input.
BinaryRewriter::BinaryRewriter(int argc, char **binaryFile) {
    //build ast and cfg.
    initialize(argc, binaryFile);
}

// Pass the binary to the frontend.
void BinaryRewriter::initialize(int argc, char **binaryFile) {
    // Call frontend to parse the file, save it in the private variable.    
    binaryProject = frontend(argc, binaryFile);
    // Extract the SgAsmInterpretation to use when building the cfg
    std::vector<SgAsmInterpretation*> asmInterpretations = SageInterface::querySubTree<SgAsmInterpretation>(binaryProject);    
    // build a cfg as well.
    rose::BinaryAnalysis::ControlFlow cfgAnalyzer;
    cfgAnalyzer.build_block_cfg_from_ast(asmInterpretations.back(), *blockCfg);
}


void BinaryRewriter::selectRegisterAllocation() {

}

void BinaryRewriter::selectInstructionScheduling() {

}

// Does the actual traversal and applies transformations to the binary.
void BinaryRewriter::transformBinary() {
    //This function will traverse the block cfg. 
    
    for (std::pair<CfgVIter, CfgVIter> VPair = vertices(*blockCfg);
          VPair.first != VPair.second; ++VPair.first) {
    ;
    }
     
}



