/* Header file for binaryRewriter.cpp */

#ifndef BINARY_REWRITER_H
#define BINARY_REWRITER_H

//Framework headers
#include "binaryDebug.hpp"
#include "cfgHandler.hpp"
#include "mipsISA.hpp"
#include "symbolicRegisters.hpp"
#include "naiveTransform.hpp"
#include "linearScan.hpp"
#include "listScheduler.hpp"
#include "binaryChanger.hpp"

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
        //Selects naive or optimized transform.
        void useNaiveTransform();
        void useOptimizedTransform();
        //Configure register allocation
        void selectRegisterAllocation();
        //Configure instruction scheduling
        void selectInstructionScheduling();
        //enable debugg printing.
        void setDebug(bool);
        /* Function that is to be transformed */
        void functionSelect(std::list<std::string>* functionNames);

        /**********************************************************************
        * Traversal functions. 
        **********************************************************************/
        //Function to begin rewriting
        void transformBinary();

        /**********************************************************************
        * Binary manipulation. 
        **********************************************************************/
        //Add instruction
        void insertInstruction(SgAsmStatement*); 
        //Remove current instruction
        void removeInstruction(); 
        //Next instruction, copies the currently inspected instruction.
        void saveInstruction();
        //Virtual function that the user can change in his framework extension.
        virtual void transformDecision(SgAsmMipsInstruction*);

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

        /**********************************************************************
        * Private variables
        **********************************************************************/
        //Pointer to the project AST 
        SgProject* binaryProjectPtr;
        /* Cfghandler pointer */
        CFGhandler* cfgContainer;
        /*  Binary changer pointer. */
        binaryChanger* sizeHandler;
        //Shadow statement list. This list will be swaped with the statementlist
        //att the end of traversing a basic blocks statement list. (vector)
        SgAsmStatementPtrList* shadowStatementListPtr;
        //Current instruction being inspected.
        SgAsmMipsInstruction* inspectedInstruction;
        //number of decisions made
        int decisionsMade;
        /* Is debugging enabled */
        bool debugging;
        /* boolean to determine if to use naive or optimized transform */
        bool useOptimized;
        /* Container pointer for storing the names of functions being transformed. */
        std::list<std::string>* functionVector;

        /**********************************************************************
        * Private Functions. 
        **********************************************************************/
        //block traversal
        void blockTraversal();
};

#endif 

