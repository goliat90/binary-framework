/*  List scheduler available for the framework. */

#ifndef LISTSCHEDULER_H
#define LISTSCHEDULER_H

/*  Includes. */
#include "rose.h"
#include <limits>

/*  Boost library. */
#include "boost/graph/breadth_first_search.hpp"

/*  Framework includes. */
#include "graphDAG.hpp"
#include "cfgHandler.hpp"
#include "binaryDebug.hpp"
#include "mipsISA.hpp"


/*  Struct for saving information regarding an instructions
    values used when scheduling. */
struct instructionVariables {
//TODO constructor for this struct
    instructionVariables():earliestStart(-1), latestStart(std::numeric_limits<int>::max()), slack(-1),
        sumOfExecutions(-1), executionTime(-1) {};
    int earliestStart;
    int latestStart;
    int slack;
    int sumOfExecutions;
    int executionTime;
};

/*  Typedef. */
typedef std::map<SgAsmMipsInstruction*, instructionVariables> nodeMap;
typedef std::pair<SgAsmMipsInstruction*, instructionVariables> nodeMapPair;

class listScheduler {
    public:
        /*  Constructor. */ 
        listScheduler(CFGhandler*);
        /*  Call to start scheduling. */
        void performScheduling();
        /*  Enable debuging. */
        void setDebuging(bool);

    private:
        /*  Hidden default constructor. */
        listScheduler(); 
        /*  Function that handles scheduling a basic block. */
        void scheduleBlock(SgAsmBlock*);
        /*  Initialize the map containing the variables for
            the instructions used by the list scheduler. */
        void initializeListVariables();

        /*  Private variables. */
        /*  CFG handler object pointer. */
        CFGhandler* cfgObject;
        /*  graph DAG object. */
        graphDAG* DAGobject;
        /*  Debuging variable. */
        bool debuging;

        //TODO create a storage medium for the instruction information, struct?
        /*  Storage medium to map the instructions. */
        std::map<SgAsmMipsInstruction*, instructionVariables> variableMap;
};


/*  Using boost breadth first search visitor class.
    With it i can use boost already implemented algorithm
    to traverse the graphs and perform calculations for
    priority information. */
class listForwardBFSVisitor : public boost::default_bfs_visitor {
    public:
        /*  Constructor, pass debug mode and possibly direction. */
        listForwardBFSVisitor(nodeMap* varMap, bool mode)
            {variableMapPtr = varMap; visitorDebuging = mode;};
        /*  Initialize vertex function. */
        void initialize_vertex(DAGVertexDescriptor, const frameworkDAG&);
        /*  On each examined edge. */
//        void examine_edge(DAGEdgeDescriptor, const frameworkDAG&);
        /*  Discover vertex call. */
        void discover_vertex(DAGVertexDescriptor, const frameworkDAG&);
        /* set debug mode. */
        void setDebugMode(bool mode) {visitorDebuging = mode;} ;
        /*  Set the traversal mode, forward or backward. */
        //void setForwardTraversal() {backwardTraversal = false; };
        //void setBackwardTraversal() {backwardTraversal = true; };
        /*  Function to pass a reference to the variableMap pointer. */
        //void passVariableMapReference(nodeMap*);
    private:
        /*  Constructor. */
        listForwardBFSVisitor();
        /*  Private variables. */
        /*  debug print mode. */
        bool visitorDebuging;
        /*  Indicator if it is a backward or forward traversal. */
        //bool backwardTraversal;
        /*  Have a pointer to the container for all the info. */
        nodeMap* variableMapPtr;
};

class listBackwardBFSVisitor : public boost::default_bfs_visitor {
    public:
        /*  Constructor, pass debug mode and possibly direction. */
        listBackwardBFSVisitor(nodeMap* varMap, DAGVertexDescriptor* root, bool mode)
            {variableMapPtr = varMap; rootPtr = root; visitorDebuging = mode;};
        /*  Initialize vertex function. */
        //void initialize_vertex(DAGVertexDescriptor, const frameworkDAG&);
        /*  On each examined edge. */
//        void examine_edge(DAGEdgeDescriptor, const frameworkDAG&);
        /*  Discover vertex call. */
        void discover_vertex(DAGVertexDescriptor, const frameworkDAG&);
        /* set debug mode. */
        void setDebugMode(bool mode) {visitorDebuging = mode;} ;
        /*  Set the traversal mode, forward or backward. */
        //void setForwardTraversal() {backwardTraversal = false; };
        //void setBackwardTraversal() {backwardTraversal = true; };
        /*  Function to pass a reference to the variableMap pointer. */
        //void passVariableMapReference(nodeMap*);
    private:
        /*  Constructor. */
        listBackwardBFSVisitor();
        /*  Private variables. */
        /*  debug print mode. */
        bool visitorDebuging;
        /*  Indicator if it is a backward or forward traversal. */
        //bool backwardTraversal;
        /*  Have a pointer to the container for all the info. */
        nodeMap* variableMapPtr;
        /*  Pointer to the root node. */
        DAGVertexDescriptor* rootPtr;
};

#endif 

