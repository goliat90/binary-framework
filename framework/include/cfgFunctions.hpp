/* Functions related to control flow graph.  */
#ifndef CFGFUNCTIONS_H
#define CFGFUNCTIONS_H

/**********************************************************************
* Includes.
**********************************************************************/
/* Framework */


#include "rose.h"
/* std::map  */
#include <map>
/* Boost includes. Adjacency list with propertymaps*/
#include <boost/graph/adjacency_list.hpp>

/**********************************************************************
* Typedefs.
**********************************************************************/
typedef rose::BinaryAnalysis::ControlFlow::Graph CFG;
typedef boost::graph_traits<CFG>::vertex_iterator CFGVIter;
typedef boost::graph_traits<CFG>::edge_iterator CFGEIter;
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
        /*  Extract a specific function from the whole program cfg.
            Returns a sub cfg that contains only the specified function.
            The cfg is build by adding the nodes(blocks) that belong to
            the function. */
        void createFunctionCFG(CFG*, std::string);
        /* Allowed to transform this instruction */
        bool isForbiddenInstruction(SgAsmInstruction*); 
        /* Check if an instruction has a new address */
        bool hasNewAddress(rose_addr_t);
        /* Get the new address of an instruction */
        rose_addr_t getNewAddress(rose_addr_t);
        /* return pointer to function cfg */
        CFG* getFunctionCFG();
        /* return pointer to program CFG */
        CFG* getProgramCFG();
    private:
/**********************************************************************
* Private Variables.
**********************************************************************/
        /* CFG pointers, for program and function cfg */
        CFG* programCFG;
        CFG* functionCFG;
        /* Name of the function that the function cfg is based on */
        std::string functionName;
        /*  Mapping between an old and new address, used later when correcting
            branches and rewriting addresses */
        std::map<rose_addr_t, rose_addr_t> instructionMap;
        /* Track forbidden instructions to transform, only for this selected
            function that is being transformed, search vector with std::find */
        std::vector<SgAsmInstruction*> forbiddenInstruction;
        /* Activation instructions, are forbidden by users to transform
            but might have to be modified by the framework, consider other storage
            than vector. */
        std::vector<SgAsmInstruction*> activationInstruction;
        /* first is the activationrecord, second is the deactivation record */
        std::pair<SgAsmInstruction*, SgAsmInstruction*> activationPair;
/**********************************************************************
* Private Functions.
**********************************************************************/
        /* Finds activation records in the functioncfg */
        void findActivationRecords();
        /* Find lowest address and highest address in the function cfg */
        void findAddressRange();
};

 

#endif
