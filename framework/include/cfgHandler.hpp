/* Functions related to control flow graph.  */
#ifndef CFGHANDLER_H
#define CFGHANDLER_H

/**********************************************************************
* Includes.
**********************************************************************/
/* Framework */
#include "mipsISA.hpp"

#include "rose.h"
/* std::map  */
#include <map>
/* Numerical limits */
#include <limits>
/* Boost includes. Adjacency list with propertymaps*/
#include <boost/graph/adjacency_list.hpp>

/**********************************************************************
* Typedefs.
**********************************************************************/
typedef rose::BinaryAnalysis::ControlFlow::Graph CFG;
typedef boost::graph_traits<CFG>::vertex_iterator CFGVIter;
typedef boost::graph_traits<CFG>::edge_iterator CFGEIter;
typedef boost::graph_traits<CFG>::out_edge_iterator CFGOEIter;
typedef boost::graph_traits<CFG>::in_edge_iterator CFGIEIter;
typedef boost::graph_traits<CFG>::vertex_descriptor CFGVertex;
//map type for the property map in the cfg that contains the basic blocks.
typedef boost::property_map<CFG, boost::vertex_name_t>::type basicBlockPropertyMap;


/*******************************************************************************
* Class containing information that is needed to perform transformations.
* Contains address spectrums, branch targets, activation records which
* are not allowed to be transformed.
*******************************************************************************/
class CFGhandler {
    public:
/**********************************************************************
* Public Functions.
**********************************************************************/
        /* Inital setup of the program cfg and this object */
        void initialize(SgProject*);
        /*  Extract a specific function from the whole program cfg.
            Returns a sub cfg that contains only the specified function.
            The cfg is build by adding the nodes(blocks) that belong to
            the function. */
        void createFunctionCFG(std::string);
        /*  Set debugging flag. */
        void setDebugging(bool mode) {debugging = mode;};
        /* Allowed to transform this instruction */
        bool isForbiddenInstruction(SgAsmMipsInstruction*); 
        /* Check if an instruction has a new address */
        bool hasNewAddress(rose_addr_t);
        /* Get the new address of an instruction */
        rose_addr_t getNewAddress(rose_addr_t);
        /* return pointer to function cfg */
        CFG* getFunctionCFG();
        /* return pointer to program CFG */
        CFG* getProgramCFG();
        /* Get the activation record pair*/
        std::pair<SgAsmMipsInstruction*, SgAsmMipsInstruction*> getActivationRecord();
        /*  Get activation instruction. */
        std::set<SgAsmMipsInstruction*>* getActivationInstructions() {return &activationInstruction;};
        /*  Get deactivation instructions. */
        std::set<SgAsmMipsInstruction*>* getDeactivationInstructions() {return &deactivationInstruction;};
        /*  Functions to get pointers to the entry and exit block. */
        SgAsmBlock* getEntryBlock() {return entryBlock;};
        //TODO remove the single exitblock function.
        SgAsmBlock* getExitBlock()  {return exitBlock;};
        std::set<SgAsmBlock*>* getExitBlocks() {return &exitBlocks; };
        
    private:
/**********************************************************************
* Private Variables.
**********************************************************************/
        /* CFG pointers, for program and function cfg */
        CFG* programCFG;
        CFG* functionCFG;
        /*  Debuging mode variable. */
        bool debugging;
        /* Name of the function that the function cfg is based on */
        std::string functionName;
        /* Variable to remember the address range the transformed instruction
            is withing. First = highest, second = lowest. */
        std::pair<rose_addr_t, rose_addr_t> addressRange;
        /*  Mapping between an old and new address, used later when correcting
            branches and rewriting addresses */
        std::map<rose_addr_t, rose_addr_t> instructionMap;
        /* Track forbidden instructions to transform, only for this selected
            function that is being transformed, search vector with std::find */
        std::set<SgAsmInstruction*> forbiddenInstruction;
        /* Activation instructions, are forbidden by users to transform
            but might have to be modified by the framework, consider other storage
            than vector. */
        //TODO consider changing this to a set instead. so i cant have duplicates.
        //TODO reason is if i have a single block then i will scan it twice from
        //TODO both directions. 
        //TODO i need to change this structure since if i have multiple exit blocks
        //TODO i will potentially have more than two activation record instructions.
        //TODO this i will need to separate int two different containers,
        //TODO one for activation and one for deactivation. 
        //TODO if i do that then i can remove the activation pair.
        std::set<SgAsmMipsInstruction*> activationInstruction;
        std::set<SgAsmMipsInstruction*> deactivationInstruction;
        /* first is the activationrecord, second is the deactivation record */
        //TODO remove this later
        std::pair<SgAsmMipsInstruction*, SgAsmMipsInstruction*> activationPair;
        /*  Pointers to entry and exit block. */
        SgAsmBlock* entryBlock;
        //TODO i can possibly have several exit blocks.
        std::set<SgAsmBlock*> exitBlocks;
        //TODO remove this exitBlock ptr.
        SgAsmBlock* exitBlock;
/**********************************************************************
* Private Functions.
**********************************************************************/
        /*  Determine the entry block and exit block of the function. */
        void findEntryAndExitBlocks();
        /* Finds activation records in the functioncfg */
        //TODO remake this function to use information gathered by findEntryAndExitBlocks,
        //TODO just let it iterate blocks found by the mentioned instruction.
        void findActivationRecordsNew();
        //TODO remove old function code and rename.
        //void findActivationRecords();
        /* Find lowest address and highest address in the function cfg */
        void findAddressRange();
};

 

#endif
