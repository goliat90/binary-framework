/* Header file for binaryRewriter.cpp */

#ifndef BINARY_REWRITER_H
#define BINARY_REWRITER_H

//Framework headers

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
        //Function to begin rewriting
        void traverseBinary();
        //Add instruction
        void insertInstruction(SgAsmStatement*); //need an argument here.
        //Remove current instruction
        void removeInstruction(); 
        //Next instruction, copies the currently inspected instruction.
        void saveInstruction();
        //Function that provides symbolic register labels.
        void generateSymbolicRegister(); //need to check how operands/registers are typed.
        //Virtual function that the user can change in his framework extension.
        virtual void transformDecision(SgAsmStatement*);
        //Print statistics.
        void printInformation();

    private:
        //typedefs
        typedef rose::BinaryAnalysis::ControlFlow::Graph Cfg;
        typedef boost::graph_traits<Cfg>::vertex_iterator CfgVIter;
        //map type for the property map in the cfg that contains the basic blocks.
        typedef boost::property_map<Cfg, boost::vertex_name_t>::type basicBlockMap;

        //Private variables.
        //Pointer to the project AST 
        SgProject* binaryProjectPtr;
        //Control flow graph pointer.
        Cfg* CfgPtr;
        //Shadow statement list. This list will be swaped with the statementlist
        //att the end of traversing a basic blocks statement list.
        SgAsmStatementPtrList* shadowStatementListPtr;
        //Current instruction being inspected.
        SgAsmStatement* inspectedInstruction;
        //number of decisions made
        int decisionsMade;

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

