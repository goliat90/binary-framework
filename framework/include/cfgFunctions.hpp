/* Functions related to control flow graph.  */
#ifndef CFGFUNCTIONS_H
#define CFGFUNCTIONS_H

/* includes */
#include "rose.h"
/* std::map  */
#include <map>

/**********************************************************************
* Typedefs.
**********************************************************************/
typedef rose::BinaryAnalysis::ControlFlow::Graph CFG;
typedef boost::graph_traits<CFG>::vertex_iterator CFGVIter;
//map type for the property map in the cfg that contains the basic blocks.
typedef boost::property_map<CFG, boost::vertex_name_t>::type basicBlockPropertyMap;



/*  Extract a specific function from the whole program cfg.
    Returns a sub cfg that contains only the specified function.
    The cfg is build by adding the nodes(blocks) that belong to
    the function. */
CFG* createFunctionCFG(CFG*, std::string);
 
    

#endif
