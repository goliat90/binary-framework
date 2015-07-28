/*  List scheduler available for the framework. */

#ifndef LISTSCHEDULER_H
#define LISTSCHEDULER_H

/*  Includes. */
#include "rose.h"

/*  Boost library. */
#include "boost/graph/breadth_first_search.hpp"

/*  Framework includes. */
#include "graphDAG.hpp"
#include "cfgHandler.hpp"
#include "binaryDebug.hpp"




/*  Struct for saving information regarding an instructions
    values used when scheduling. */
struct instructionVariables {
//TODO constructor for this struct
    instructionVariables():earliestStart(-1), latestStart(-1), slack(-1),
        sumOfExecutions(-1), executionTime(-1) {};
    int earliestStart;
    int latestStart;
    int slack;
    int sumOfExecutions;
    int executionTime;
};

/*  Typedef. */
typedef std::map<SgAsmMipsInstruction*, instructionVariables> nodeMap;

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
        /*  Initialize vertex function. */
        void initialize_vertex(DAGVertexDescriptor, const frameworkDAG&);
        /* set debug mode. */
        void setDebugMode(bool mode) {debuging = mode;} ;
        /*  Set the traversal mode, forward or backward. */
        //void setForwardTraversal() {backwardTraversal = false; };
        //void setBackwardTraversal() {backwardTraversal = true; };
        /*  Function to pass a reference to the variableMap pointer. */
        void passVariableMapReference(nodeMap*);
    private:
        /*  Private variables. */
        /*  debug print mode. */
        bool debuging;
        /*  Indicator if it is a backward or forward traversal. */
        //bool backwardTraversal;
        /*  Have a pointer to the container for all the info. */
        nodeMap* variableMapPtr;
};

#endif 

