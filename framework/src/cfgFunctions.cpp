/* CFG functions */

#include "cfgFunctions.hpp"


/* Makes a cfg for a specific function */
CFG* createFunctionCFG(CFG* programCFG, std::string functionName) {
    /* New cfg variable */
    CFG* functionCFG = new CFG;
    /* track visited blocks */
    std::map<SgAsmBlock *, bool> visitedBlock;
    /* track mapping between programcfg and functioncfg vertices */
    std::map<CFG::vertex_descriptor, CFG::vertex_descriptor> vertexMap;
    /* keep track of which blocks are included in the functioncfg */
    std::map<CFG::vertex_descriptor, bool> copiedVertex;

    /*  Get the property maps of the program cfg and the function cfg   */
    boost::property_map<CFG, boost::vertex_name_t>::type programPropMap = get(boost::vertex_name, *programCFG);
    boost::property_map<CFG, boost::vertex_name_t>::type functionPropMap = get(boost::vertex_name, *functionCFG);

    /*  Go through all vertices (blocks) and find blocks belonging to the
        function that is being extracted.   */
    for(std::pair<CFGVIter, CFGVIter> verticePair = vertices(*programCFG);
        verticePair.first != verticePair.second; ++verticePair.first) {
        /*  Get the SgAsmBlock from the propertymap and check the name*/
        SgAsmBlock* basicBlock = get(boost::vertex_name, *programCFG, *verticePair.first); 
        /*  See if a vertex has been visited    */
        if (visitedBlock.find(basicBlock) == visitedBlock.end()) {
            /* Set the block as visited */
            visitedBlock.insert(std::pair<SgAsmBlock*, bool>(basicBlock, true));
            /*  Retrieve the enclosing function, i.e the function the block
                belongs to. */
            SgAsmFunction* blockFunction = basicBlock->get_enclosing_function();
            /* Check the functions name with the passed name */
            if (functionName.compare(blockFunction->get_name()) == 0) {
                /*  The block belongs to the desired function add it to
                    the function CFG. */
                CFG::vertex_descriptor newVertex = add_vertex(*functionCFG);
                /* set the values of the property map in the new CFG */
                put(functionPropMap, newVertex, basicBlock);
                /*  Save the vertex descriptor so i know which vertexes
                    have been copies */
                copiedVertex.insert(std::pair<CFG::vertex_descriptor, bool>(*verticePair.first , true));
                /* Add both vertices to the vertexmap */
                vertexMap.insert(std::pair<CFG::vertex_descriptor, CFG::vertex_descriptor>
                (*verticePair.first, newVertex));
            } else {
                /*  The block does not belong to a desired function, set it
                    as not copied */
                copiedVertex.insert(std::pair<CFG::vertex_descriptor, bool>(*verticePair.first, false));
            }
        }
    }
    /* All vertices have been created in the new cfg with their properties.
        Now add all edges within the program */
    for(std::pair<CFGEIter, CFGEIter> edgePair = edges(*programCFG); edgePair.first != edgePair.second;
        ++edgePair.first) {
        /* retrieve source and target vertex descriptors */
        CFG::vertex_descriptor sourceVertex = source(*edgePair.first, *programCFG);
        CFG::vertex_descriptor targetVertex = target(*edgePair.first, *programCFG);

        /*  Check if the source and target vertices have been copied.
            That means that the edge is within the program */
        if (copiedVertex[sourceVertex] && copiedVertex[targetVertex]) {
            /* create the edge between the vertices  */
            CFG::vertex_descriptor newSourceV = vertexMap[sourceVertex];
            CFG::vertex_descriptor newTargetV = vertexMap[targetVertex];
            /* Add the edge to the cfg. */
            add_edge(newSourceV, newTargetV, *functionCFG);
        }
    }
    return functionCFG;
}



