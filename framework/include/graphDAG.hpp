/*  Code for DAG representation used by the framework. */

#ifndef GRAPH_DAG_H
#define GRAPH_DAG_H

/*  Framework includes. */
#include "mipsISA.hpp"
#include "cfgHandler.hpp"

/*  Includes */
#include "rose.h"

/*  Typedef for the DAG graph. */

/*  Properties for the properties of the vertices. */
typedef boost::property<boost::vertex_name_t, SgAsmInstruction*> vertexProperties_DAG;

/*  Properties for the edges in the graph. The weight is intended to be the latency
    of the instruction. */
    //TODO perhaps change this property to what the dependency is instead, or add it,
    //TODO could move latency to some other place.
typedef boost::property<boost::edge_weight_t, int> edgeProperties_DAG;

/*  The graph itself. */
typedef boost::adjacency_list<  boost::setS,
                                boost::vecS,
                                boost::bidirectionalS,
                                vertexProperties_DAG,
                                edgeProperties_DAG > frameworkDAG;

/*  typedefs for iterators and such. */
typedef boost::graph_traits<frameworkDAG>::vertex_iterator DAGVIter;
typedef boost::graph_traits<frameworkDAG>::vertex_descriptor DAGVertexDescriptor;
typedef boost::graph_traits<frameworkDAG>::edge_iterator DAGEIter;


//TODO Create some kind of definition for resources that is used when building the dag.
/*  Enums used for tracking which resources are being used. */
namespace DAGresources {
    enum resourceEnum {
        /*  Register names */
        zero,
        at,
        v0,v1,
        a0,a1,a2,a3,
        t0,t1,t2,t3,t4,t5,t6,t7,
        s0,s1,s2,s3,s4,s5,s6,s7,
        t8,t9,
        k0,k1,
        gp,
        sp,
        fp,
        ra,
        /*  Special registers, accumulator. */
        highAcc, lowAcc,
        /*  Memory resource. I do not think i can change the order which load and
            stores are encountered. */
        memoryResource
    };
}

class graphDAG {
    public:
        /*  Constructor. */
        graphDAG(SgAsmBlock*);
        /*  Build DAGS. */
        void buildDAGs();

    private:
        /*  Hide default constructor. */
        graphDAG();
        /*  build a DAG for block. */
        //TODO this will probably be a forward build and a backward version.
        void buildForwardDAG();
        void buildBackwardDAG();
        /*  Resource definition function. */
        //TODO determine arguments, is probably the node and the resource checked.
        void resourceDefined();
        /*  Resource use function. */
        //TODO determine arguments, is probably the node and the resource checked.
        void resourceUsed();

        /*  Variables. */
        /*  DAG graphs. */
        frameworkDAG* forwardDAG;
        frameworkDAG* backwardDAG;
        /*  Basic block that is being schedules. */
        SgAsmBlock* basicBlock;
        //TODO some kind of storage for the variables used when scheduling. map with instructions as keys?

        /*  Map for which definition is set. */
        std::map<DAGresources::resourceEnum, DAGVertexDescriptor>  definitionMap;
        /*  Map for which vertices are using a resource. */
        std::map<DAGresources::resourceEnum, std::set<DAGVertexDescriptor> > useMap;
};


#endif

