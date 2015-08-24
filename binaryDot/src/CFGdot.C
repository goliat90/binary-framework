//#include <iostream>
//#include <fstream>
//#include "rose.h"
//#include "BinaryControlFlow.h"
#include "binaryDotGraph.h"

// predeclarations of functions
void subCFG (CFG &largecfg, CFG &subcfg, string function, int);

int main( int argc, char * argv[] ) {
    /*  Check that the number of arguments are enough.
        First argument is path to binary cfg binary
        Second will be the path to the binary beign analyzed.
        Third will be the function name to generate cfg of.
        Fourth will be the degree of neighbours.
    */
    if (4 != argc) {
        std::cout << "Invalid amount of arguments." << std::endl
            << "Arg1: Path to binary being analyzed" << std::endl
            << "Arg2: name of function being analyzed" << std::endl
            << "Arg3: Degree of neighbour inclusion." << std::endl;

        exit(0);
    }
    /*  Transfer the char function name to a string variable. */
    string functionName = string(argv[2]);
    string stringDegree = string(argv[3]);
    int intDegree = strtol(&argv[3][0], NULL, 0);
    /*  Printout function name. */
    std::cout << "Function to generate graph for is called "
        << functionName << " with neighbour degree " << stringDegree << std::endl;

    // Use the frondend to generate AST.
    // generates two lines of printout
    SgProject* project = frontend(argc-2,argv);

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
    //subCFG(*bigcfg, *subcfg, "simpleAdditionTMR");
    subCFG(*bigcfg, *subcfg, functionName, intDegree);

    //call the graph maker function when i have a cfg.
    //BinaryDotGenerator(*bigcfg, "main" , "main.dot", true);
    //BinaryDotGenerator(*subcfg, "SimpleAdditionTMR" , "simpleAdditionTMR.dot", true);
    BinaryDotGenerator(*subcfg, functionName , (functionName + stringDegree + ".dot"), true);

    //
    return 0;
}



// function to extract a subcfg for a specific function from a cfg.
void subCFG (CFG &largecfg, CFG &subcfg, string function, int nDegree)
{
    //keep track of visited vertices(blocks).
    //SgAsmBlock is key, bool to determine if visited.
    map<SgAsmBlock *, bool> visitedBlock;
    //keep track of which vertex in the old cfg represents which in the new cfg.
    map<CFG::vertex_descriptor, CFG::vertex_descriptor> vertexMap;
    //track blocks that are included in the new cfg.
    map<CFG::vertex_descriptor, bool> copiedVertex;
    

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
                //save the vertex_descriptor so i know which vertexes were copied.
                copiedVertex.insert(std::pair<CFG::vertex_descriptor, bool>(*verticePair.first, true));
                //add both vertexes to the vertexMap.
                vertexMap.insert(std::pair<CFG::vertex_descriptor, CFG::vertex_descriptor>(*verticePair.first, newVertex));
            } else {
                //the block does not belong to main, it is not copied.
                copiedVertex.insert(std::pair<CFG::vertex_descriptor, bool>(*verticePair.first, false));
            }
        }
    }
    // At this point all vertices for the function in question have
    // been added. Now we add all neighbouring vertices that have edges that
    // jump into or out of the function. Were getting one degree of neighbours.

    for(int i=0; i < nDegree; i++) {
        // Before finding the neighbours add all the current copied vertexes
        // to the copiedNeighbour map to avoid duplicate vertice copies.
        map<CFG::vertex_descriptor, bool> copiedNeighbourV (copiedVertex);
        
        std::cout << "Copying neighbours of degree " << i+1 << endl;

        for(std::pair<CFGEIter, CFGEIter> edgePair = edges(largecfg);
            edgePair.first != edgePair.second; ++edgePair.first) {
            //get the target and source vertices.
            CFG::vertex_descriptor sourceVertex = source(*edgePair.first, largecfg); 
            CFG::vertex_descriptor targetVertex = target(*edgePair.first, largecfg); 
            //check if either the target or source vertex is a copied vertex
            //OBS! NEED TO THINK ABOUT IF A NEIGHBOUR HAS ALREADY BEEN COPIED!!
            if (copiedVertex[targetVertex] && !copiedNeighbourV[sourceVertex]) {
                //The edge goes into the function. We want the block it jumps from
                //in the graph as well. Add the source vertex and its block.
                std::cout << "found target vertex neighbour" << endl;
                CFG::vertex_descriptor neighbourV = add_vertex(subcfg);
                //set the vertex_name property in the new cfg.
                subPmap[neighbourV] = largePmap[sourceVertex];
                //save the vertex in the neighbour copied map avoiding duplicate copying.
                //copiedNeighbourV.insert(std::pair<CFG::vertex_descriptor, bool>(sourceVertex, true);
                copiedNeighbourV[sourceVertex] = true;
                //add the neighbour vertex to the vertex map
                vertexMap.insert(std::pair<CFG::vertex_descriptor, CFG::vertex_descriptor>(sourceVertex, neighbourV));
                //
            } else if (copiedVertex[sourceVertex] && !copiedNeighbourV[targetVertex]) {
                //The edge goes out of the function. We want the block it jumps to
                //in the graph as well. Add the target vertex and its block.
                std::cout << "found target source neighbour" << endl;
                CFG::vertex_descriptor neighbourV = add_vertex(subcfg);
                //set the vertex_name in the new cfg.
                subPmap[neighbourV] = largePmap[targetVertex];
                //save the vertex in the neighbour copied map so we do not copy it twice.
                //copiedNeighbourV.insert(std::pair<CFG::vertex_descriptor, bool>(targetVertex, true));
                copiedNeighbourV[targetVertex] = true;
                //add the neighbour vertex to the vertex map
                vertexMap.insert(std::pair<CFG::vertex_descriptor, CFG::vertex_descriptor>(targetVertex, neighbourV));
            }
        }

        //Change the neighbouring vertices false value in the copiedvertex map to true.
        for(std::map<CFG::vertex_descriptor, bool>::iterator neighbourIter = copiedNeighbourV.begin();
            neighbourIter != copiedNeighbourV.end(); neighbourIter++) {
            //Transfer the copied neighbours to the copied vertex map.
            //copiedVertex.insert(std::pair<CFG::vertex_descriptor, bool>(neighborIter->first, true));
            copiedVertex[neighbourIter->first] = neighbourIter->second;
        }
    }

    //All relevant vertices have been added to the new cfg with their properties.
    //now go through the edges and add all edges that connect between relevant blocks.
    //This is done by checking the source and target of an edge_descriptor,
    //if both source and target are relevant blocks then it is copied.
    for(std::pair<CFGEIter, CFGEIter> edgePair = edges(largecfg); edgePair.first != edgePair.second;
        ++edgePair.first) {
        //get the source and target vertex descriptors.
        CFG::vertex_descriptor sourceVertex = source(*edgePair.first, largecfg);
        CFG::vertex_descriptor targetVertex = target(*edgePair.first, largecfg);

        //check the source vertex and target vertex of the edge.
        if (copiedVertex[sourceVertex] && copiedVertex[targetVertex]) {
            //both target and source vertex were copied. Add the edge.
            //Get retrieve source and targe vertices in the new cfg.
            CFG::vertex_descriptor newSourceV = vertexMap[sourceVertex];
            CFG::vertex_descriptor newTargetV = vertexMap[targetVertex];
            //add the edge to the cfg.
            add_edge(newSourceV, newTargetV, subcfg);
        }
    }
}


