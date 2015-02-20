//#include <iostream>
//#include <fstream>
//#include "rose.h"
//#include "BinaryControlFlow.h"
#include "binaryDotGraph.h"

//using namespace rose;


int main( int argc, char * argv[] ) 
   {
    // Use the frondend to generate AST.
    // Will it handle binary or should it be changed?
    SgProject* project = frontend(argc,argv);
    //retrieve the assembler interpertation.
 //   std::vector<SgAsmInterpretation*> interps = SageInterface::querySubTree<SgAsmInterpretation>(project);

    //Create CFG from the SgProject, code from binaryCFGTraversalTutorial.C
   // BinaryAnalysis::ControlFlow cfg_analyzer;
    //BinaryAnalysis::ControlFlow::Graph* cfg = new BinaryAnalysis::ControlFlow::Graph;
   // CFG* cfg = new CFG;

    //build the CFG
   // cfg_analyzer.build_block_cfg_from_ast(interps.back() , *cfg);

    //call the graph maker function when i have a cfg.
    //BinaryDotGenerator(cfg, "test" , "test", true);

    //
    return 0;
   }

