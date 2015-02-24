//#include <iostream>
//#include <fstream>
//#include "rose.h"
//#include "BinaryControlFlow.h"
#include "binaryDotGraph.h"



int main( int argc, char * argv[] ) 
   {
    // Use the frondend to generate AST.
    // generates two lines of printout
    SgProject* project = frontend(argc,argv);

    //Find the main function to reduce the cfg.
    //SgFunctionDeclaration* main = SageInterface::findMain(project);

    //retrieve the assembly interpretation from the main.
    std::vector<SgAsmInterpretation*> test = SageInterface::querySubTree<SgAsmInterpretation>(project);

    //i have many assembler interpretations now. Should find relevant ones (main):

    //retrieve the assembler interpertation.
    std::vector<SgAsmInterpretation*> interps = SageInterface::querySubTree<SgAsmInterpretation>(project);

    //Create CFG from the SgProject, code from binaryCFGTraversalTutorial.C
    rose::BinaryAnalysis::ControlFlow cfg_analyzer;
    rose::BinaryAnalysis::ControlFlow::Graph* cfg = new rose::BinaryAnalysis::ControlFlow::Graph;

    //build the CFG, interps.back = last element in the vector.
    cfg_analyzer.build_block_cfg_from_ast(interps.back() , *cfg);
    //cfg_analyzer.build_block_cfg_from_ast(test.back(), *cfg);

    //call the graph maker function when i have a cfg.
    BinaryDotGenerator(*cfg, "main" , "main.dot", false);

    //
    return 0;
   }

