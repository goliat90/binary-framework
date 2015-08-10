/*  Code for DAG representation used by the framework. */

#ifndef GRAPH_DAG_H
#define GRAPH_DAG_H

/*  Framework includes. */
#include "mipsISA.hpp"
#include "cfgHandler.hpp"
#include "binaryDebug.hpp"

/*  Includes */
#include "rose.h"
#include "boost/bimap.hpp"
#include "boost/bimap/set_of.hpp"
#include "boost/bimap/multiset_of.hpp"
#include "boost/graph/graph_utility.hpp"
#include "boost/graph/transpose_graph.hpp"

/*  Typedef for the DAG graph. */

/*  Properties for the properties of the vertices. */
/*  NOTE! vertex_index is not used because it seems to be
    used/instantiated by something and becomes unusable, it breaks the execution. */
/*  vertex_index1_t is used for numbering the nodes. It is used to ensure
    that the identity of the nodes are the same in backward and forward DAGS. */
/*  vertex_index2_t is used when debuging and names the strings with the number
    they are encountered in the graph and the instruction mnemonic. */
typedef boost::property<boost::vertex_name_t, SgAsmMipsInstruction*, 
        boost::property<boost::vertex_index1_t, int, 
        boost::property<boost::vertex_index2_t, std::string> > > vertexProperties_DAG;

/*  Properties for the edges in the graph. The weight is intended to be the latency
    of the instruction. */
namespace edgeDependency {
    //TODO add edge for enforcing sink and root node. Should limit scheduler.
    enum edgeType{
        /*  The sinkRootDependency is used to ensure that the first and last
            instruction in a block become root and sink nodes in the DAG.
            This property is set on edges enforcing this. */
        sinkRootDependency,
        WAW,
        RAW,
        WAR
    };
}

/*  This property is used to identify dependencies when scheduling. */
typedef boost::property<boost::edge_weight_t, edgeDependency::edgeType> edgeProperties_DAG;

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
typedef boost::graph_traits<frameworkDAG>::out_edge_iterator DAGOEIter;
typedef boost::graph_traits<frameworkDAG>::in_edge_iterator DAGIEIter;
typedef boost::graph_traits<frameworkDAG>::edge_descriptor DAGEdgeDescriptor;

//propertymaps
typedef boost::property_map<frameworkDAG, boost::vertex_index2_t>::type vertexIndexNameMap;
typedef boost::property_map<frameworkDAG, boost::vertex_index1_t>::type vertexNumberMap;
typedef boost::property_map<frameworkDAG, boost::vertex_name_t>::type vertexInstructionMap;

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

/*  Typedefs for maps. */
typedef std::map<DAGresources::resourceEnum, DAGVertexDescriptor> definitionContainer;
//typedef std::map<DAGresources::resourceEnum, std::set<DAGVertexDescriptor> > useContainer;
typedef boost::bimap<boost::bimaps::multiset_of<DAGresources::resourceEnum>, boost::bimaps::multiset_of<DAGVertexDescriptor> > useContainer;

/*  DAG graph class. */
class graphDAG {
    public:
        /*  Constructor. */
        graphDAG(SgAsmBlock*);
        /*  Build DAGS. */
        void buildDAGs();
        /*  Enable debuging. */
        void setDebuging(bool);
        /*  Return pointer to the backwardDAG. */
        frameworkDAG* getBackwardDAG() {return backwardDAG;};
        /*  Return pointer to the forwardDAG. */
        frameworkDAG* getForwardDAG() {return forwardDAG;};
        /*  Return a reference to the first instruction vertex. */
        DAGVertexDescriptor* getForwardDAGRoot() {return &forwardDAGRootVertice;};
        /*  Return a reference to the last instruction vertex. */
        DAGVertexDescriptor* getBackwardDAGRoot() {return &backwardDAGRootVertice;};

    private:
        /*  Hide default constructor. */
        graphDAG();
        /*  build a DAG for block. */
        void buildForwardDAG();
        void buildBackwardDAG();
        /*  Resource definition function. */
        void resourceDefined(DAGVertexDescriptor*, DAGresources::resourceEnum);
        /*  Resource use function. */
        void resourceUsed(DAGVertexDescriptor*, DAGresources::resourceEnum);
        /*  Gets resource enum from register name. */
        DAGresources::resourceEnum getRegisterResource(mipsRegisterName);
        /*  Handle instructions that use accumulator register. */
        bool involvesAcc(MipsInstructionKind);
        /*  Function that handles the accumulator resource. */
        void manageAccumulator(DAGVertexDescriptor*, MipsInstructionKind);
        /*  Check if an instruction is a jump instruction. */
        bool isBranchInstruction(SgAsmStatement*);
        /*  Identify the root vertices of the DAGs. */
        void findRootVertices();

        /*  Variables. */
        /*  Debuging variable. */
        bool debuging;
        /*  DAG graphs. */
        frameworkDAG* forwardDAG;
        frameworkDAG* backwardDAG;
        /*  Basic block that is being schedules. */
        SgAsmBlock* basicBlock;
        /*  Pointer to last instruction in block. */
        SgAsmMipsInstruction* lastInstruction;
        DAGVertexDescriptor backwardDAGRootVertice;
        /*  Pointer to first instruction in block. */
        SgAsmMipsInstruction* firstInstruction;
        DAGVertexDescriptor forwardDAGRootVertice;

        /*  Map for which definition is set. */
        std::map<DAGresources::resourceEnum, DAGVertexDescriptor>  definitionMap;
        /*  BiMap for which vertices are using a resource. */
        useContainer useBiMap;
};


#endif

