#ifndef LIVEVARIABLE_H
#define LIVEVARIABLE_H
/*  Live-variable analysis.
*   Given a control flow graph the definition and use of variables
*   are computed for each basic block. Then live-variable analysis is applied
*   to the control flow graph, lastly the live intervals are computed.
*/

/*  Includes    */
#include "cfgHandler.hpp"
#include "mipsISA.hpp"
#include "binaryDebug.hpp"

/*  Boost includes  */
#include "boost/bimap.hpp"
#include "boost/bimap/set_of.hpp"
#include "boost/bimap/multiset_of.hpp"

#include "boost/dynamic_bitset.hpp"
#include "boost/graph/depth_first_search.hpp"

/*  Typedefs    */
/*  pair containing the definition and usage of variables in a block, order
    is first = definition, second = usage.
    When used for IN and OUT calculations, first = IN, second = OUT. */
typedef std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<> > bitPair;

/*  Typedef for the interval maps, maps interval(int) to symbolic(unsigned). */
typedef boost::bimap<boost::bimaps::multiset_of<int>, boost::bimaps::set_of<unsigned> > intervalMap;

//testing dfs visitor
class liveDFSVisitor : public boost::default_dfs_visitor {
    public:
        /* Custom function on discovery of vertex */
        void discover_vertex(CFGVertex, const CFG& g) const;
        /* Pointer to store a list of the visited order */
        std::list<SgAsmBlock*>* dfsBlockOrder;
        /*  Function to pass a reference to the list */
        void passListReference(std::list<SgAsmBlock*>*);
};

class liveVariableAnalysisHandler {
    public:
    /*  Constructor */
    liveVariableAnalysisHandler(CFG*);
    /*  Returns some kind of structure with live intervals
        or is a query function perhaps  */
    //TODO add function head.
    /*  Query function to to get information if variables are live
        at a certain instruction. */
    void livenessAtInstruction();
    /*  Execute the live range analysis */
    void performLiveRangeAnalysis();
    /*  enable debuging    */
    void setDebug(bool);

    private:
    /*  Hide default constructor    */
    liveVariableAnalysisHandler();
    /*  Definition and use function, calculates on blocks. */
    void computeDefAndUseOnBlocks();
    /*  Function to determine in and out on basic blocks. */
    void computeInOutOnBlocks();
    /*  Function to compute OUT on live-range analysis on block level */
    boost::dynamic_bitset<> computeOutOnBlock(SgAsmBlock*);
    /*  Help functions to add and remove ENTRY and EXIT blocks */
    void addEntryExit();
    void removeEntryExit();
    /*  Initializes the IN and OUT before computation */
    void initializeInOutOnBlocks();
    /*  Calculate IN and OUT on for instructions */
    void computeInstructionInOut();
    /*  Checks source registers for use */
    void instructionUsageAndDefinition(SgAsmStatement*, bitPair*);
    /*  Count the symbolic registers present */
    void countSymbolicRegisters();
    /*  Function that determines a correct DFS order that will be used in
        the live-range analysis */
    void determineOrderOfDFS();
    /*  Live interval function, finds the intervals,
        depth first search traversal.   */
    void buildLiveIntervals();

    /*  Variables   */
    /*  Switch debuging on and off. */
    bool debuging;
    /*  Pointer to function cfg */
    CFG* functionCFG;
    /*  Block pointer for the basic block that is the entry block */
    SgAsmBlock* cfgRootBlock;
    CFGVertex rootVertex;
    /*  ENTRY and EXIT vertices and their blocks. */
    CFG::vertex_descriptor ENTRY;
    SgAsmBlock* blockENTRY;
    CFG::vertex_descriptor EXIT;
    SgAsmBlock* blockEXIT;
    /*  Number of variables found, used to set the width of the bitset. */
    int numberOfVariables;

    /*  Storage for def and use of basic blocks, block ptr is used as key. */
    std::map<SgAsmBlock*, bitPair> defuseBlockMap;
    /*  Storage for def and use of instructions */
    std::map<SgAsmMipsInstruction*, bitPair> defuseInstructionMap;
    /*  map between the symbolic register and their bit */
    std::map<unsigned, int> symbolicToBit;

    /*  Storage for IN and OUT of basic blocks. */
    std::map<SgAsmBlock*, bitPair> inoutBlockMap;
    /*  Storage for IN and OUT for individual instructions. */
    std::map<SgAsmMipsInstruction*, bitPair> inoutInstructionMap;
    /*  The DFS order of the instructions in a list */
    std::list<std::pair<int, SgAsmMipsInstruction*> > DFSInstructionOrder;

    /*  Storage representation for live intervals. */
        //TODO determine a representation for the intervals, ordered by the
        //increasing start point.

    //int is the start point, easy to access. can be sorted.
    //the pair contains the end point and the symbolic 
    //TODO perhaps use a map so it will sort and be easy to find later?
    //TODO map does not allow duplicate keys. can it be a problem?
    //TODO believe so if two ranges end in the same place or begin.
    //TODO a multimap could probably be the solution instead. 
    intervalMap startPointBiMap;
    intervalMap endPointBiMap;

};

#endif 

