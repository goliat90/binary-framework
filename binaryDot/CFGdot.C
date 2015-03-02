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
    //BinaryDotGenerator(*bigcfg, "main" , "main.dot", true);
    BinaryDotGenerator(*subcfg, "main" , "main.dot", true);

    //
    return 0;
}



// function to extract a subcfg for a specific function from a cfg.
void subCFG (CFG &largecfg, CFG &subcfg, string function)
{
    //keep track of visited vertices(blocks).
    //SgAsmBlock is key, bool to determine if visited.
    map<SgAsmBlock *, bool> visitedBlock;
    //keep track of which vertex in the old cfg represents which in the new cfg.
    map<CFG::vertex_descriptor, CFG::vertex_descriptor> vertexMap;

    //get the property map for original cfg and the new cfg, why do i need ::type?!
    //this makes it possible to extract the SgAsmBlock through the map by giving the vertex_property as key.
    boost::property_map<CFG, boost::vertex_name_t>::type largePmap = get(boost::vertex_name, largecfg);
    boost::property_map<CFG, boost::vertex_name_t>::type subPmap = get(boost::vertex_name, subcfg);

    //Go through all the vertices(blocks) by starting in one block
    for(std::pair<CFGVIter, CFGVIter> verticePair = vertices(largecfg);
        verticePair.first != verticePair.second; ++verticePair.first) {
        //Extract the SgAsmBlock from vertex_name property in the graph.
        //SgAsmBlock* basicBlock = get(boost::vertex_name, largecfg, *verticePair.first);
        SgAsmBlock* basicBlock = largePmap[*verticePair.first];
        //check a vertex if it has not been visited.
        if (visitedBlock.find(basicBlock) == visitedBlock.end()) {
            //does the vertex/block belong to the main function?
            visitedBlock.insert(std::pair<SgAsmBlock*, bool>(basicBlock, true));
            //Retrieve the enclosing function.
            SgAsmFunction* blockFunction = basicBlock->get_enclosing_function();
            //get the function name and compare it to the given function string
            if (blockFunction->get_name() == function) {
                //the blocks belongs to function main, add it to the new cfg.
                CFG::vertex_descriptor newVertex = add_vertex(subcfg);
                //set the values of vertex_name propertymaps in the new cfg.
                subPmap[newVertex] = largePmap[*verticePair.first];
                //add both vertexes to the vertexMap.
                vertexMap.insert(std::pair<CFG::vertex_descriptor, CFG::vertex_descriptor>(*verticePair.first, newVertex));
            }
        }
                //if so add it to relevant Blocks, or copy it over right away to subcfg?
                //visit the blocks are in the edge list, check if they belong to main.
                //if the block in the edge belongs to main then add it to the edge list.
        //if it does not belong to main then set as visited and continue with the next block.
    }
    //All relevant vertices have been added to the new cfg with their properties.
    //now go through the edges and add all edges that connect between relevant blocks.

    //traverse the edges.
}


