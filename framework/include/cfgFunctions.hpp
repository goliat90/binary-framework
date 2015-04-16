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
        /* Allowed to transform this instruction */
        /* Get the branch instructions target address */
    private:
        /* Keep track of where a branch jumps, this is for the whole program */
        /* Track the spectrum of addresses in the function, */
        /* Track forbidden instructions to transform, only for the
            selected function that is being transformed */
};

/*  Extract a specific function from the whole program cfg.
    Returns a sub cfg that contains only the specified function.
    The cfg is build by adding the nodes(blocks) that belong to
    the function. */
CFG* createFunctionCFG(CFG*, std::string);
 

#endif
