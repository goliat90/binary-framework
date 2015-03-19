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
        //Default constructor.
        BinaryRewriter() {};
        //Constructor with file
        BinaryRewriter(int, char**);
        //Takes argc and argv then creates an ast and cfg.
        void initialize(int ,char**);
        //Configure register allocation
        void selectRegisterAllocation();
        //Configure instruction scheduling
        void selectInstructionScheduling();
        //Function to begin rewriting
        void traverseBinary();

        //Add instruction
        void insertInstruction(SgAsmStatement*); //need an argument here.
        //
        void insertInstruction(MipsInstructionKind, std::string); //need an argument here.
        //
        //void insertInstruction(SgAsmStatement*); //need an argument here.
        //

        //Remove current instruction
        void removeInstruction(); 
        //Next instruction, copies the currently inspected instruction.
        void saveInstruction();
        //Get the instruction enum of the currently inspected instruction.
        MipsInstructionKind getInstructionKind();
        //Return the operand list of the current instruction, result and source regs.
        SgAsmExpressionPtrList getInstructionOperands();
        //return the mnemonic string
        std::string getInstructionMnemonic();

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
        SgAsmMipsInstruction* inspectedInstruction;
        //number of decisions made
        int decisionsMade;

        // -------- Functions --------
        //block traversal
        void blockTraversal();
        //function traversal
        void functionTraversal();
};

#endif 

