/* Header file for binaryRewriter.cpp */

#ifndef BINARY_REWRITER_H
#define BINARY_REWRITER_H

//Framework headers
#include "userTransformer.hpp"

// Boost lib headers
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vector_as_graph.hpp>

// Rose headers
#include "rose.h"

/* Class declaration */
class BinaryRewriter {
    public:
        //Constructor with file
        BinaryRewriter(int, char**);
        //Configure register allocation
        void selectRegisterAllocation();
        //Configure instruction scheduling
        void selectInstructionScheduling();
        //Function to start rewriting
        void transformBinary();

    private:
        //typedefs
        typedef rose::BinaryAnalysis::ControlFlow::Graph Cfg;
        typedef boost::graph_traits<Cfg>::vertex_iterator CfgVIter;
        //map type for the property map in the cfg that contains the basic blocks.
        typedef boost::property_map<Cfg, boost::vertex_name_t>::type basicBlockMap;

        //Pointer to the project AST 
        SgProject* binaryProjectPtr;
        //CFG implementation
        Cfg* blockCfgPtr;
        //userTransformer object
        userTransformer* transformPtr = NULL;

        // -------- Functions --------
        //Constructor, hidding it to force use of the other constructor
        //requiring argc and argv.
        BinaryRewriter();
        //Takes argc and argv then creates an ast and cfg.
        void initialize(int ,char**);
        //block traversal
        void blockTraversal();
        //function traversal
        void functionTraversal();
        //give the framework a transformer object.
        void passTransformer(userTransformer&);
};

#endif 

