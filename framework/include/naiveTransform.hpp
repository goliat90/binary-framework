#ifndef NAIVETRANSFORM_H
#define NAIVETRANSFORM_H
/* 
* Naive transformation that is lazy when creating an executable binary.
* It will in a simple way do register allocation, not regarding performance
* at all, only considering creating an correct binary.
*/

/* Includes */
#include <queue>
#include "rose.h"
/* Boost includes. Adjacency list with propertymaps*/
#include <boost/graph/adjacency_list.hpp>

/* Framework includes */
#include "mipsISA.hpp"
#include "cfgHandler.hpp"
#include "symbolicRegisters.hpp"
#include "binaryDebug.hpp"

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
        /*  boolean confirming use of special register acc/hi/lo */
        bool usesAcc;
        /*  Counter for the offset used in store/load */
        uint64_t offset = 0;
        /* maximum number of symbolic registers used */
        int maximumSymbolicsUsed;
        /*  Structure to store the available register names */
        std::set<mipsRegisterName> hardRegisters;
        /* number of used hard registers in a region */
        int usedHardRegs;

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
        void regionAllocation(std::list<SgAsmStatement*>*); //, SgAsmStatementPtrList*);
        /*  Help function that will return hard registers for exchange.
            It will also create a offset for the load and store instructions */
        mipsRegisterName getHardRegister();
        /* init function for hardRegisters set */
        void initHardRegisters();
        /* Function to check if a function uses accumulator register */
        bool usesAccumulator(MipsInstructionKind);
        /* Adds specific instructions to handle saving and restoring the accumulator register */
        void saveAccumulator(std::list<SgAsmStatement*>*, mipsRegisterName);
};

#endif
