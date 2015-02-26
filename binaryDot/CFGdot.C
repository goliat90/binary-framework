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

    //retrieve the assembly interpretation.
    std::vector<SgAsmInterpretation*> test = SageInterface::querySubTree<SgAsmInterpretation>(project);
    
    //see how many assembly interpertations we have.
    //std::cout << "SgAsmInterpertations: " << test.size() << "\n";

    //find all the SgAsmFunction nodes in the SgAsmInterpretation
    //std::vector<SgAsmFunction*> test2 = SageInterface::querySubTree<SgAsmFunction>(test.back());

    //std::cout << "SgAsmFunctions: " << test2.size() << "\n";

    //print out the function names.
    //SgAsmFunction* onlyMain;
    //int i = 0;
    //for (std::vector<SgAsmFunction*>::iterator iter = test2.begin(); iter != test2.end(); iter++) {
        //print out the function name.
        //std::cout << i << ": " << (*iter)->get_name() << "\n";
    //    if ((*iter)->get_name() == "main") {
    //        std::cout << "main found" << "\n";
    //        onlyMain = *iter;
    //        std::cout << onlyMain->get_name() << "\n";
    //    }
    //    i++;
    //}

    //Create CFG from the SgProject, code from binaryCFGTraversalTutorial.C
    rose::BinaryAnalysis::ControlFlow cfg_analyzer;
    rose::BinaryAnalysis::ControlFlow::Graph* cfg = new rose::BinaryAnalysis::ControlFlow::Graph;

    //build the CFG, interps.back = last element in the vector.
    cfg_analyzer.build_block_cfg_from_ast(test.back(), *cfg);

    //call the graph maker function when i have a cfg.
    BinaryDotGenerator(*cfg, "main" , "main.dot", true);

    //
    return 0;
   }

