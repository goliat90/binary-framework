
/*  Include */
#include "graphDAG.hpp" 


/*  Constructor. */
graphDAG::graphDAG(SgAsmBlock* block) {
    /*  Save the pointer to the function cfg. */
    basicBlock = block;
}


/*  Builds the DAGs used when scheduling. */
void graphDAG::buildDAGs() {
    /*  Build the backward DAG. */
    buildBackwardDAG();
    /*  Build the forward DAG. */
    buildForwardDAG();
}

/*  Constructs a DAG traversing forward. */
void graphDAG::buildForwardDAG() {

}

/*  Constructs DAG traversing backwards. */
void graphDAG::buildBackwardDAG() {
    /*  Clear the sets. */
    definitionMap.clear();
    useMap.clear();
    /*  Create framework DAG object. */
    backwardDAG = new frameworkDAG;
    /*  Get the statement list. */
    SgAsmStatementPtrList& stmtList = basicBlock->get_statementList();

    /*  iterate through the statement list backwards. */
    for(SgAsmStatementPtrList::reverse_iterator rIter = stmtList.rbegin();
        rIter != stmtList.rend(); ++rIter) {
        /*  For each statement, check definitions and then use if it's
            an instruction. To do that the instruction is decoded. */
        if (V_SgAsmMipsInstruction == (*rIter)->variantT()) {
            /*  Cast to correct pointer. */
            SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*rIter);
            /*  Decode. */
            instructionStruct currentInst = decodeInstruction(mips);
            /*  check destination registers. */

            /*  Check source registers. */
        }
        
    }
    /*  The backward dag has been built. */
}

/*  Function that handles definition of resources. */
void graphDAG::resourceDefined() {

}
/*  Resource defined. */
/*  Backward pass does define then use. */
/*
    for each newnode(instruction)
    if (resource is defined AND no use of the resource)
        add WAW arc between newnode and resource defined entry (previous instruction);

    for each (use definition entry in resource uselist in ascending order) do
    {
        add RAW arc between newnode and uselist entry
        remove remove uselist entry from resource uselist.
    }
    insert newnode as the resource definition entry.
*/

/*  Resource used. */
void graphDAG::resourceUsed() {

}
/*
    if (resource is defined)
        add WAR arc between newnode and resource defined entry.
    add newnode as a uselist entry in resource uselist.
*/
        
