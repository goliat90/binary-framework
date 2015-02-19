

#include "rose.h"
#include "BinaryControlFlow.h"

int main( int argc, char * argv[] ) 
   {
    // Use the frondend to generate AST.
    // Will it handle binary or should it be changed?
    SgProject* project = frontend(argc,argv);

    // Run internal consistency tests on AST
    //AstTests::runAllTests(project);

    //Create CFG from the SgProject, code from binaryCFGTraversalTutorial.C
    BinaryAnalysis::ControlFlow cfg_analyzer;
    BinaryAnalysis::ControlFlow::Graph* cfg = new BinaryAnalysis::ControlFlow::Graph;

    //this should build the graph but i need the node/binary ast here?
    //cfg_analyzer.build_block_cfg_from_ast(SgNode here , *cfg);

    //call the graph maker function when i have a cfg.

    //
    return 0;
   }

