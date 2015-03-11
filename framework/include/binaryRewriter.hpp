/* Header file for binaryRewriter.cpp */

#ifndef BINARY_REWRITER_H
#define BINARY_REWRITER_H


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

        //Pointer to the project AST 
        SgProject *binaryProject;
        //CFG implementation
        Cfg* blockCfg;

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

};





#endif 

