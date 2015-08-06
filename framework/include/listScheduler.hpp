/*  List scheduler available for the framework. */

#ifndef LISTSCHEDULER_H
#define LISTSCHEDULER_H

/*  Includes. */
#include "rose.h"
#include <limits>

/*  Boost library. */
#include "boost/graph/breadth_first_search.hpp"
#include "boost/bimap.hpp"
#include "boost/bimap/set_of.hpp"

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
    //TODO consider adding mipsinstruction pointer? Or the vertex descriptor instead?
    //TODO might not need this if i change nodeMap to a bimap.
    DAGVertexDescriptor forwardNodeRef;
};

/*  Comparator here. */
struct priorityCompareFunctor {
    bool operator()(const instructionVariables& instA, const instructionVariables& instB) {
        /*  Check if there is any of the instructions are on the critical path. */
        if (0 == instA.slack && 0 == instB.slack) {
            /*  Both instructions are on critical path, check next variable. */
            if (instA.earliestStart == instB.earliestStart) {
               /*   Equal EST values, check next variable. */ 
               if (instA.slack == instB.slack) {
                    /*  Equal slack, check next variable. */
                    if (instA.maximumDelayToLeaf == instB.maximumDelayToLeaf) {
                        /*  Equal maximum delay to leaf, check next variable.
                            Comparing execution times, higher execution times are prioritized. */
                        return instA.executionTime > instB.executionTime;

                    } else if (instA.maximumDelayToLeaf > instB.maximumDelayToLeaf) {
                        /*  instA has higher delay and therefore higher priority. */
                        return true;
                    } else {
                        /*  instB has the higher priority. */
                        return false;
                    }   
               } else if (instA.slack < instB.slack) {
                    /*  instA has a smaller slack and has higher priority. */
                    return true;
               } else {
                    /*  instB has smaller slack, therefore higher priority. */
                    return false;
               }   
            } else if (instA.earliestStart < instB.earliestStart) {
                /*  instA has a lower EST than instB and har higher priority. */
                return true;
            } else {
                /*  If none of the above cases match then it means that instB
                    has lower EST than instA. */
                return false;
            }
        } else if (0 == instA.slack) {
            /*  InstA is on critical path and should be before instB which is
                not on critical path. */
            return true;
        } else {
            /*  InstB is on critical path and should be before instA which is
                not on critical path. */
            return false;
        }
    }
};

/*  Typedef. */
typedef std::map<DAGVertexDescriptor, instructionVariables> nodeMap;
typedef std::pair<DAGVertexDescriptor, instructionVariables> nodeMapPair;

//TODO Seems like i need a special comparator for instruction variables.
//TODO with the bimap i can skip having a vertex variable in the struct.
typedef boost::bimap<DAGVertexDescriptor, boost::bimaps::set_of<instructionVariables, priorityCompareFunctor> > nodeBiMap;
typedef nodeBiMap::left_value_type leftNodePair;
typedef nodeBiMap::right_value_type rightNodePair;

//typedef std::map<DAGVertexDescriptor, SgAsmMipsInstruction*> nodeToInstructionMap;

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
        /*  Help function to determine which execution time an instruction has. */
        int getExecutionTime(SgAsmMipsInstruction*);
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
        void forwardListScheduling(graphDAG*, std::list<DAGVertexDescriptor>&);

        /*  Private variables. */
        /*  CFG handler object pointer. */
        CFGhandler* cfgObject;
        /*  Debuging variable. */
        bool debuging;

        /*  Storage medium to map the instructions.
            This map is used when calculating the different variables used when
            scheduling. The variables are then later used during scheduling
            to determine order. */
        nodeMap variableMap;
};

#endif 

