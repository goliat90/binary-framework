/* Header file for binaryRewriter.cpp */

#ifndef BINARY_REWRITER_H
#define BINARY_REWRITER_H

//Framework headers
#include "mipsISA.hpp"
#include "binaryDebug.hpp"
#include "symbolicRegisters.hpp"

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

        /**********************************************************************
        * Configuration functions
        **********************************************************************/
        //Configure register allocation
        void selectRegisterAllocation();
        //Configure instruction scheduling
        void selectInstructionScheduling();
        //enable debugg printing.
        void enableDebugg(bool);

        /**********************************************************************
        * Traversal functions. 
        **********************************************************************/
        //Function to begin rewriting
        void traverseBinary();

        /**********************************************************************
        * Binary manipulation. 
        **********************************************************************/
        //Add instruction
        void insertInstruction(SgAsmStatement*); 
        //Remove current instruction
        void removeInstruction(); 
        //Next instruction, copies the currently inspected instruction.
        void saveInstruction();
        //get the information struct for the current instruction.
        //instructionInformation getInstructionInfo();
        //Virtual function that the user can change in his framework extension.
        virtual void transformDecision(SgAsmStatement*);

        /**********************************************************************
        * Misc. 
        **********************************************************************/
        //Print statistics.
        void printInformation();
        //Print out a basic blocks instructions.
        void printBasicBlock(SgAsmBlock*);

    private:
        /**********************************************************************
        * Typedefs. 
        **********************************************************************/
        typedef rose::BinaryAnalysis::ControlFlow::Graph Cfg;
        typedef boost::graph_traits<Cfg>::vertex_iterator CfgVIter;
        //map type for the property map in the cfg that contains the basic blocks.
        typedef boost::property_map<Cfg, boost::vertex_name_t>::type basicBlockMap;

        /**********************************************************************
        * Private variables
        **********************************************************************/
        //Pointer to the project AST 
        SgProject* binaryProjectPtr;
        //Register dictionary.
        const RegisterDictionary* mipsRegisters; 
        //Control flow graph pointer.
        Cfg* CfgPtr;
        //Shadow statement list. This list will be swaped with the statementlist
        //att the end of traversing a basic blocks statement list. (vector)
        SgAsmStatementPtrList* shadowStatementListPtr;
        //Current instruction being inspected.
        SgAsmMipsInstruction* inspectedInstruction;
        //number of decisions made
        int decisionsMade;

        /**********************************************************************
        * Functions. 
        **********************************************************************/
        //block traversal
        void blockTraversal();
        //function traversal
        void functionTraversal();
        // convert a basic block vector to a list.
        void bbVectorToList(SgAsmBlock*);
        
};

#endif 

