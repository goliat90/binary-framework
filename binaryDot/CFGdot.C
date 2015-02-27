//#include <iostream>
//#include <fstream>
//#include "rose.h"
//#include "BinaryControlFlow.h"
#include "binaryDotGraph.h"

// predeclarations of functions
void subCFG (CFG &largecfg, CFG &subcfg, string function);

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
    rose::BinaryAnalysis::ControlFlow::Graph* bigcfg = new rose::BinaryAnalysis::ControlFlow::Graph;
    rose::BinaryAnalysis::ControlFlow::Graph* subcfg = new rose::BinaryAnalysis::ControlFlow::Graph;

    //build the CFG, interps.back = last element in the vector.
    cfg_analyzer.build_block_cfg_from_ast(test.back(), *bigcfg);

    // The cfg i have is to big, i want a subset of it. More specific i want
    // a graph only over main.
    subCFG(*bigcfg, *subcfg, "main");

    //call the graph maker function when i have a cfg.
    BinaryDotGenerator(*bigcfg, "main" , "main.dot", true);

    //
    return 0;
}



// function to extract a subcfg for a specific function from a cfg.
void subCFG (CFG &largecfg, CFG &subcfg, string function)
{
    //keep track of visited vertices(blocks).
    //SgAsmBlock is key, bool to determine if visited.
    map<SgAsmBlock *, bool> visitedBlock;

    //might need another that is tracks blocks relevant to the function
    map<SgAsmBlock *, bool> relevantBlock;

    //Go through all the vertices(blocks) by starting in one block
    //and traverse the edges to visit the next blocks. 

    //get the vertices from the cfg

    for(std::pair<CFGVIter, CFGVIter> verticePair = vertices(largecfg);
        verticePair.first != verticePair.second; ++verticePair.first) {
        //Extract the SgAsmBlock from vertex_name property in the graph.
        //passing the cfg and the 
        SgAsmBlock* basicBlock = get(boost::vertex_name, largecfg, *verticePair.first);
        //check a vertex if it has not been visited.
    //    if (visitedBlock.find(verticePair.first) == visitedBlock.end()) 
            //does the vertex belong to main?
                //if so add it to relevant Blocks, or copy it over right away to subcfg?
                //visit the blocks are in the edge list, check if they belong to main.
                //if the block in the edge belongs to main then add it to the edge list.
        
        
        //if it does not belong to main then set as visited and continue with the next block.
    }
}


