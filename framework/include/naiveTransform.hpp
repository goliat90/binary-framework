#ifndef NAIVETRANSFORM_H
#define NAIVETRANSFORM_H
/* 
* Naive transformation that is lazy when creating an executable binary.
* It will in a simple way do register allocation, not regarding performance
* at all, only considering creating an correct binary.
*/

/* Includes */
#include "rose.h"
/* Boost includes. Adjacency list with propertymaps*/
#include <boost/graph/adjacency_list.hpp>

/* Framework includes */
#include "mipsISA.hpp"
#include "cfgHandler.hpp"
#include "symbolicRegisters.hpp"


/* Object class for naive transformations. */
class naiveHandler{
    public:
        /* Constructor */
        naiveHandler(CFGhandler* cfg);

        /* Function for applying the naive transformation  */
        void applyTransformation();
    private:
        /* Private variables */
        CFGhandler* cfgContainer;
        /* maximum number of symbolic registers used */
        int maximumSymbolicsUsed;
        /* instruction region list  */
        //std::list<SgAsmStatement*> regionList;

        /* Functions */
        //Hidding default constructor. I want a cfghandler for this object
        naiveHandler() {};
        /*  Checks the amount of stack space needed by finding the maximum
            amount of symbolic register used. */
        void determineStackModification();
        /*  Help function that increments register use for special registers. */
        void specialInstructionUse(MipsInstructionKind, int*);
        /*  Transforms a basic block. Inserts SW/LW instructions and replaces
            the symbolic registers with real registers. */
        void naiveBlockTransform(SgAsmBlock*);
        
        /* Transform region. A region of inserted instructions is transformed. */
        void regionAllocation(std::list<SgAsmStatement*>*, SgAsmStatementPtrList*);
};

#endif
