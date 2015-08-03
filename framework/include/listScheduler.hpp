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
    /*  Constructor. */
    instructionVariables():earliestStart(-1), latestStart(std::numeric_limits<int>::max()), slack(-1),
        maximumDelayToLeaf(-1), executionTime(-1) {};
    int earliestStart;
    int latestStart;
    int slack;
    int maximumDelayToLeaf;
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
        void initializeListVariables(graphDAG*);
        /*  This function will propogate the EST values by traversing the
            forward DAG. */
        void propagateEST(graphDAG*);
        /*  This function does the same as the propagateEST but it propagates
            the LST value instead. It also calculates slack which is the difference
            between LST and EST. */
        void propagateLST(graphDAG*);
        /*  Calculates the maximum delay to the most distant leaf. This should help
            balance the progress on the paths in the DAG more evenly. Even though
            all DAGs will have one leaf only this metric will hopefully be usable. */
        void maximumDelayToLeaf(graphDAG*);
        /*  Calculate the slack variable. It is just the difference between the
            EST and LST, can be calculated by iterating over the variable map. */
        void calculateSlack();
        /*  Handles the scheduling of instructions. */
        void forwardListScheduling(graphDAG*);

        /*  Private variables. */
        /*  CFG handler object pointer. */
        CFGhandler* cfgObject;
        /*  Debuging variable. */
        bool debuging;

        //TODO create a storage medium for the instruction information, struct?
        /*  Storage medium to map the instructions. */
        nodeMap variableMap;
};

#endif 

