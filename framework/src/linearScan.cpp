/* Source code for the framework linear scan */



/* includes */
#include "linearScan.hpp"


/*  Constructor */
linearScanHandler::linearScanHandler(CFGhandler* passedCfgObject){
    /*  Set Default values */
    debuging = false;
    /*  Save the pointer to the cfg handler */
    cfgHandlerPtr = passedCfgObject;
    /*  Create the live variable analysis object */
    liveRangeHandler = new liveVariableAnalysisHandler(cfgHandlerPtr->getFunctionCFG());
    /*  Initialize the freeRegister set */
    /*  t0-t7 */
    freeRegisters.insert(t0);
    freeRegisters.insert(t1);
    freeRegisters.insert(t2);
    freeRegisters.insert(t3);
    freeRegisters.insert(t4);
    freeRegisters.insert(t5);
    freeRegisters.insert(t6);
    freeRegisters.insert(t7);
    /*  t8-t9 */
    freeRegisters.insert(t8);
    freeRegisters.insert(t9);
    /*  extra register to test. */
    freeRegisters.insert(a2);
    freeRegisters.insert(a3);
}


/*  Activate or deactivate debuging printout*/
void linearScanHandler::selectDebuging(bool mode) {
    debuging = mode;
    /*  pass it to the liverange analysis */
    liveRangeHandler->setDebug(mode);
    
}

/*  Applies the linear scan */
void linearScanHandler::applyLinearScan() {
    /*  Go through the cfg and replace the register that linear scan will use. */
    if (debuging) {
        std::cout << "Replacing physical registers used by linear scan." << std::endl;
    }
    replaceHardRegisters();

    /*  Get live-range analysis done before performing register allocation. */
    if (debuging) {
        std::cout << "Starting the live-range analysis." << std::endl;
    }
    liveRangeHandler->performLiveRangeAnalysis();

}

/*  Replace the registers used by linear scan with symbolic names.
    Each physical register will be replaced with one symbolic register that
    will be used in the whole CFG. This could be considered a greedy method
    since it will result in long live ranges for these registers. */
void linearScanHandler::replaceHardRegisters() {
    /*  mapping between physical register and symbolic name */
    std::map<mipsRegisterName, registerStruct> physicalToSymbolic;
    /*  CFG pointer */
    CFG* functionCFG = cfgHandlerPtr->getFunctionCFG();
    /*  Iterate through the vertices */
    for(std::pair<CFGVIter, CFGVIter> vertPair = vertices(*functionCFG);
        vertPair.first != vertPair.second; ++vertPair.first) {
        /*  Get the block pointer */
        SgAsmBlock* block = get(boost::vertex_name, *functionCFG, *vertPair.first);
        /*  Retrieve the statement list */
        SgAsmStatementPtrList& stmtList = block->get_statementList();
        /*  Go through the statement list and look for the specific registers */
        for(SgAsmStatementPtrList::iterator stmtIter = stmtList.begin();
            stmtIter != stmtList.end(); ++stmtIter) {
            /*  Verify that the statement is a mips instruction */
            if (V_SgAsmMipsInstruction == (*stmtIter)->variantT()) {
                /*  Cast to mips instruction pointer */
                SgAsmMipsInstruction* mips = isSgAsmMipsInstruction(*stmtIter);
                /*  Get the operand list and check for instructions. */
                SgAsmExpressionPtrList& opList = mips->get_operandList()->get_operands(); 
                /*  Go through the operands */
                for(int i = 0; i < opList.size(); i++) {
                    /*  Get pointer to the expression */
                    SgAsmExpression* expr = opList.at(i);
                    /*  Check if the expression is a register */
                    if (V_SgAsmDirectRegisterExpression == expr->variantT()) {
                        /*  Decode the register */
                        registerStruct reg = decodeRegister(expr);
                        /*  Determine if the register should be replaced or not.
                            Also check if it has been replaced already. */
                        if (1 == freeRegisters.count(reg.regName)) {
                            /*  The register should be replaced, check if it has a symbolic or not */
                            if (0 == physicalToSymbolic.count(reg.regName)) {
                                /*  First time it has been encountered, generate a symbolic */
                                registerStruct symReg = generateSymbolicRegister();
                                /*  Debug printout */
                                if (debuging) {
                                    std::cout << "Replaced a register with sym_" << std::dec
                                                << symReg.symbolicNumber << " saved to map." << std::endl;
                                }
                                /*  Insert values into the map */
                                physicalToSymbolic.insert(std::pair<mipsRegisterName, registerStruct>(reg.regName, symReg));
                                /*  Build the register expression */
                                SgAsmDirectRegisterExpression* DRE = buildRegister(symReg);
                                /*  Replace the register expression with the new expression. */
                                opList.at(i) = DRE;
                            } else {
                                /*  The register should be replaced, it has been replaced before. */
                                registerStruct usedSymReg = physicalToSymbolic.find(reg.regName)->second;
                                /*  Debug printout */
                                if (debuging) {
                                    std::cout << "Replaced a register with sym_" << std::dec
                                                << usedSymReg.symbolicNumber << std::endl;
                                }
                                /*  Get the register expression */
                                SgAsmDirectRegisterExpression* usedDRE = buildRegister(usedSymReg);
                                /*  Replace the expression. */
                                opList.at(i) = usedDRE;
                            }
                        }
                    }
                }
            }
        }
    }
}


/*  Algorithms for linear scan. */ 
void linearScanHandler::linearScanAllocation() {
/*
Active is a list of active live intervals
active list <- empty
foreach live interval i, in order of increasing start point
    EXPIREOLDINTERVAL(i)
    if length(active) = R then
        SPILLATINTERVAL(i)
    else
        register[i] <- a register removed from pool of free registers
        add i to active, sorted by inreasing end point
*/

}


void linearScanHandler::expireOldInterval() {
/*
foreach interval j in active, in order of increasing end point
    if endpoint[i] >= startpoint[i] then
        return
    remove j from active
    add register[j] to pool of free registers
*/
}


void linearScanHandler::spillAtInterval() {
/* 
spill <- last interval in active
if endpoint[spill] > endpoint[i] then
    register[i] <- register[spill]
    location[spill] <- new stack location
    remove spill from active
    add i to active, sorted by increasing end point
else
    location[i] <- new stack location
*/

}
