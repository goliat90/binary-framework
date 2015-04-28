/******************************************************************************
* Contains information relevant to instruction which might be needed
* by the framework. Handles decoding of instructions
******************************************************************************/

/* header file */
#include "mipsISA.hpp"


/******************************************************************************
* Forward function declarations
******************************************************************************/

/********** Building instruction functions **********/
/* Builds an SgAsmMipsInstruction that can be inserted into the binary */
SgAsmMipsInstruction* buildInstruction(instructionStruct* instInfo); 
/* Build operand list */
SgAsmOperandList* buildOperandList(instructionStruct*, bool, bool, bool, bool, bool);
/* Creates a register expression */
SgAsmExpression* buildRegister(registerStruct);
/* Create a value expression, constant */
void buildValueExpression(SgAsmExpression*, instructionStruct*);
/* Create a memory expression */
void buildMemoryReference(SgAsmExpression*, instructionStruct*);

/********** Decoding instruction functions. **********/
/* Decode the instruction */
instructionStruct decodeInstruction(SgAsmMipsInstruction*);
/* operandList decoder, decodes the operands in the list. */
instructionStruct decodeOpList(SgAsmExpressionPtrList*, bool, bool, bool, bool, bool);
/* Get the format of the instructions */
instructionType getInstructionFormat(SgAsmMipsInstruction*);
/* decode register names */
registerStruct decodeRegister(SgAsmExpression*); 
/* decode value expression, a constant */
void decodeValueExpression(SgAsmExpression*, instructionStruct*);
/* decode the memory expression */
void decodeMemoryReference(SgAsmExpression*, instructionStruct*);


/********** Misc functions **********/
typedef boost::bimap<unsigned, mipsRegisterName> biRegMap;
/* initfunction for the registerName map,   */
static biRegMap initRegisterNameMap();

/* Global variables */
/* map for the register names. Maps values of registers to the enum name.  */
static biRegMap registerNameMap = initRegisterNameMap(); 


/******************************************************************************
* Build/decode instruction functions. 
******************************************************************************/
/* Builds an SgAsmMipsInstruction that can be inserted into the binary */
SgAsmMipsInstruction* buildInstruction(instructionStruct* instInfo) {
    /* Construct a mips instruction, use information from the struct. */
    SgAsmMipsInstruction* mipsInst = new SgAsmMipsInstruction;
    /* Get the statementlist reference */
    SgAsmOperandList* asmOpList; //= mipsInst->get_operandList(); 
    //SgAsmExpressionPtrList& operandList = mipsInst->get_operandList()->get_operands();
    /* depending on instruction format use right arguments for build function */
    switch (instInfo->format) {
        /* R instructions */
        case R_RD_RS_RT :{
            asmOpList = buildOperandList(instInfo, true, true, true, false, false);
            break;
        }
        case R_RD_RS_C  :{
            break;
        }
        case R_RD       :{
            break;
        }
        case R_RS_RT    :{
            break;
        }
        case R_RS       :{
            break;
        }
        case R_NOP  :{
            break;
        }
        /* I instructions  */
        case I_RD_RS_C   :{
            break;
        }
        case I_RD_MEM_RS_C:{
            break;
        }
        case I_RD_C      :{
            break;
        }
        case I_RS_RT_C   :{
            break;
        }
        case I_RS_MEM_RT_C:{
            break;
        }
        case I_RS_C      :{
            break;
        }
        /* J instructions */
        case J_C        :{
            break;
        }
        case J_RS       :{
            break;
        }
        case J_RD_RS    :{
            break;
        }
        default: {
            //The instruction is unknown.
        }
    }

    /* Set the general values of the instruction. */
    mipsInst->set_kind(instInfo->kind);
    mipsInst->set_mnemonic(instInfo->mnemonic);
    mipsInst->set_address(instInfo->address);
    /* Attach the operand list to the instruction */
    mipsInst->set_operandList(asmOpList);

    return mipsInst;
}

/* decode instruction. Calls on the R,I or J decode functions. */
instructionStruct decodeInstruction(SgAsmMipsInstruction* inst) {
    instructionStruct instStruct;
    /* Check what kind of instruction format it is.  */
    instructionType format = getInstructionFormat(inst);
    /* get the operand list */
    SgAsmExpressionPtrList* operandList = &inst->get_operandList()->get_operands();
    /* depending on instruction format use right arguments for decode function
        and save the returned instruction struct. */
    switch (format) {
        /* R instructions */
        case R_RD_RS_RT :{
            instStruct = decodeOpList(operandList, true, true, true, false, false);
            break;
        }
        case R_RD_RS_C  :{
            instStruct = decodeOpList(operandList, true, true, false, true, false);
            break;
        }
        case R_RD       :{
            instStruct = decodeOpList(operandList, true, false, false, false, false);
            break;
        }
        case R_RS_RT    :{
            instStruct = decodeOpList(operandList, false, true, true, false, false);
            break;
        }
        case R_RS       :{
            instStruct = decodeOpList(operandList, false, true, false, false, false);
            break;
        }
        case R_NOP  :{
            break;
        }
        /* I instructions  */
        case I_RD_RS_C   :
            instStruct = decodeOpList(operandList, true, true, false, true, false);
            break;
        case I_RD_MEM_RS_C: 
            instStruct = decodeOpList(operandList, true, false, false, false, true);
            break;
        case I_RD_C      :
            instStruct = decodeOpList(operandList, true, false, false, true, false);
            break;
        case I_RS_RT_C   :
            instStruct = decodeOpList(operandList, false, true, true, true, false);
            break;
        case I_RS_MEM_RT_C:
            instStruct = decodeOpList(operandList, false, true, false, false, true);
            break;
        case I_RS_C      : {
            instStruct = decodeOpList(operandList, false, true, false, true, false);
            break;
        }
        /* J instructions */
        case J_C        :
            instStruct = decodeOpList(operandList, false, false, false, true, false);
            break;
        case J_RS       :
            instStruct = decodeOpList(operandList, false, true, false, false, false);
            break;
        case J_RD_RS    : {
            instStruct = decodeOpList(operandList, true, true, false, false, false);
            break;
        }
        default: {
            //The instruction is unknown.
        }
    }
    /* Save the address of the instruction, consider other
       values that are common to all instructions. kind,mnemonic, */
    instStruct.kind = inst->get_kind();
    instStruct.mnemonic = inst->get_mnemonic();
    instStruct.address = inst->get_address();
    instStruct.format = format;

    /* Return the insturction struct from the decoding */    
    return instStruct;
}


/******************************************************************************
* Build/decode operandlist functions.
******************************************************************************/
/* Build operand list */
SgAsmOperandList* buildOperandList(instructionStruct* inst,
    bool hasRD, bool hasRS, bool hasRT, bool hasC, bool memOp) {
    /* variables */
    SgAsmOperandList* asmOpListPtr = new SgAsmOperandList;
    //instructionStruct instruction;
    /* Fill the struct with information. Check if for each type of value if it
       is present in the instruction by checking the booleans. */ 
    if (true == hasRD) {
        /* Has a destination register, extract it. */
//        registerStruct RDstruct = decodeRegister((*operandList)[opIndex]);       
//        /* Insert the register into the struct as a destination register */
//        instruction.destinationRegisters.push_back(RDstruct);
//        /* Increment the operand index */
//        opIndex++;
    }
    if (true == hasRS) {
        /* Has a rs register operand, extract it. */
//        registerStruct RSstruct = decodeRegister((*operandList)[opIndex]);
//        /* insert the registers into the struct as a source register */
//        instruction.sourceRegisters.push_back(RSstruct);
//        /* Increment the operand index */
//        opIndex++;
    }
    if (true == hasRT) {
        /* Has a rt register operand, extract it. */
//        registerStruct RTstruct = decodeRegister((*operandList)[opIndex]);       
//        /* insert the registers into the struct as a source register */
//        instruction.sourceRegisters.push_back(RTstruct);
//        /* Increment the operand index */
//        opIndex++;
    }
    if (true == hasC) {
        /* get the relevant values from the constant */
//        decodeValueExpression((*operandList)[opIndex], &instruction);
//        /* increment the operand index */
//        opIndex++;
    }
    if (true == memOp) {
        /* this is a memory instruction, extract the register and memory constant. */
//        decodeMemoryReference((*operandList)[opIndex], &instruction);       
    }
    /* return the instruction information */
    
    return asmOpListPtr;
}

/* operandList decoder, decodes the operands in the list. */
instructionStruct decodeOpList(SgAsmExpressionPtrList* operandList,
    bool hasRD, bool hasRS, bool hasRT, bool hasC, bool memOp) {
    /* variables */
    int opIndex = 0;
    instructionStruct instruction;
    /* Fill the struct with information. Check if for each type of value if it
       is present in the instruction by checking the booleans. */ 
    if (true == hasRD) {
        /* Has a destination register, extract it. */
        registerStruct RDstruct = decodeRegister((*operandList)[opIndex]);       
        /* Insert the register into the struct as a destination register */
        instruction.destinationRegisters.push_back(RDstruct);
        /* Increment the operand index */
        opIndex++;
    }
    if (true == hasRS) {
        /* Has a rs register operand, extract it. */
        registerStruct RSstruct = decodeRegister((*operandList)[opIndex]);
        /* insert the registers into the struct as a source register */
        instruction.sourceRegisters.push_back(RSstruct);
        /* Increment the operand index */
        opIndex++;
    }
    if (true == hasRT) {
        /* Has a rt register operand, extract it. */
        registerStruct RTstruct = decodeRegister((*operandList)[opIndex]);       
        /* insert the registers into the struct as a source register */
        instruction.sourceRegisters.push_back(RTstruct);
        /* Increment the operand index */
        opIndex++;
    }
    if (true == hasC) {
        /* get the relevant values from the constant */
        decodeValueExpression((*operandList)[opIndex], &instruction);
        /* increment the operand index */
        opIndex++;
    }
    if (true == memOp) {
        /* this is a memory instruction, extract the register and memory constant. */
        decodeMemoryReference((*operandList)[opIndex], &instruction);       
    }
    /* return the instruction information */
    return instruction;
}


/******************************************************************************
* Build/decode register functions.
******************************************************************************/
/* Creates a register expression */
SgAsmExpression* buildRegister(registerStruct regStruct) {
    /* Register expression, requires a register descriptor, it is
        initially created as a zero register descriptor, however minor
        needs to be set again. */
    RegisterDescriptor rd = RegisterDescriptor(mips_regclass_gpr, 0, 0, 32);
    /* Direct register expression ptr */
    SgAsmDirectRegisterExpression* directReg;

    /* If the register descriptor is not a symbolic register then
        the minor number needs to be set to the correct register */
    if (regStruct.regName != symbolic_reg) {
        /* Get the number or the regname and set it to minor */
        unsigned newMinor = registerNameMap.right.find(regStruct.regName)->second;
        rd.set_minor(newMinor);
        /* Construct the directregisterexpression with the register descriptor */
        directReg = new SgAsmDirectRegisterExpression(rd);
    } else if (regStruct.regName == symbolic_reg) {
        /* if the register is symbolic the register expression needs
            to be retrieved from the map */
    }
    /* return the register expression */
    return directReg;
}

/* decode a register operand */
registerStruct decodeRegister(SgAsmExpression* expr) {
    /* the register struct */
    registerStruct regStruct;
    //cast it to register expression
    SgAsmDirectRegisterExpression* regExpr = isSgAsmDirectRegisterExpression(expr);
    //get the register descriptor, from it i can get minor(number)
    RegisterDescriptor reg = regExpr->get_descriptor();
    //check if the register exists in the namemap
    if (registerNameMap.left.find(reg.get_minor()) != registerNameMap.left.end()) {
        //register found, get the iterator and get the register enum.
        regStruct.regName = registerNameMap.left.find(reg.get_minor())->second;
        /* If the register is zero then check if it is symbolic. */
        if (true == isSymbolicRegister(regExpr)) {
            /* Set the regName member to the symbolic enum */
            regStruct.regName = symbolic_reg;
            /* Save the symbolic register number. */
            regStruct.symbolicNumber = findSymbolicRegister(regExpr);
        }
    }
    return regStruct;
}

/******************************************************************************
* Build/decode value expression functions.
******************************************************************************/
/* decode value expression, a constant. Save values in the struct */
void decodeValueExpression(SgAsmExpression* inst, instructionStruct* instStruct) {
    /* cast the SgAsmExpression */
    SgAsmIntegerValueExpression* ve = isSgAsmIntegerValueExpression(inst); 
    /* save the constant  */
    instStruct->instructionConstant = ve->get_absoluteValue();
    /* save the significant bits, needed? */
    instStruct->significantBits = ve->get_significantBits();
}

/******************************************************************************
* Build/Decode memoryreference functions.
******************************************************************************/
/* Decode memoryreference expressions */
void decodeMemoryReference(SgAsmExpression* inst, instructionStruct* instStruct) {
    /* cast the sgasmexpression */
    SgAsmMemoryReferenceExpression* memref = isSgAsmMemoryReferenceExpression(inst);
    /* Get the size of the memory reference, 8,16,32,64 */
    SgAsmType* refType = memref->get_type();
    instStruct->memoryReferenceSize = refType->get_nBits();
    /* Get the address expression which is a binary add consisting of
       one register and one constant. */
    SgAsmBinaryExpression* binExp = isSgAsmBinaryExpression(memref->get_address());    
    /* decode the lhs(register) and the rhs(constant) expressions */
    registerStruct reg = decodeRegister(binExp->get_lhs());
    instStruct->sourceRegisters.push_back(reg);
    /* decode the constant and pass the struct as well */
    decodeValueExpression(binExp->get_rhs(), instStruct);
}


/******************************************************************************
* Return the instruction format.
******************************************************************************/
instructionType getInstructionFormat(SgAsmMipsInstruction* inst) {
    //struct to store information.
    instructionStruct instruction;
    //check the check the kind of instruction.
    switch (inst->get_kind()) {
        /********** R type instruction **********/
        //arithmetic.
        case mips_add   :
        case mips_addu  :
        case mips_sub   :
        case mips_subu  :

        //logical
        case mips_and   :
        case mips_nor   :   
        case mips_or    :   
        case mips_xor   :   

        //shift operations
        case mips_sllv  :
        case mips_srav  :
        case mips_srlv  :

        //condition, testing
        case mips_slt   :
        case mips_sltu  :

        //multiply and div
        case mips_mul   : return R_RD_RS_RT;

        //shift operations 
        case mips_sll   :   
        case mips_sra   :   
        case mips_srl   : return R_RD_RS_C;

        //multiply and div.
        case mips_div   :
        case mips_divu  :
        case mips_madd  :
        case mips_maddu :
        case mips_msub  :
        case mips_msubu :
        case mips_mult  :
        case mips_multu : return R_RS_RT;

        //accumulator ops.
        case mips_mflo  : 
        case mips_mfhi  : return R_RD;
        case mips_mthi  :
        case mips_mtlo  : return R_RS;

        //nop instruction
        case mips_nop   : return R_NOP;

        /********** I type instruction **********/
        //arithmetic
        case mips_addi  :
        case mips_addiu :

        //logical ops.
        case mips_andi  :
        case mips_ori   :   
        case mips_xori  :

        //condition, testing
        case mips_slti  :
        case mips_sltiu : return I_RD_RS_C;

        //load operations.
        case mips_lb    :
        case mips_lbu   :
        case mips_lh    :
        case mips_lhu   :
        case mips_lw    :
        case mips_lwl   :
        case mips_lwr   : return I_RD_MEM_RS_C;

        //conditional branch 
        case mips_beq   :
        case mips_bne   : return I_RS_RT_C;

        //store operations
        case mips_sb    :
        case mips_sh    :
        case mips_sw    :
        case mips_swl   :
        case mips_swr   : return I_RS_MEM_RT_C;

        //conditional branches
        case mips_bgez  :
        case mips_bgezal:
        case mips_bgtz  :
        case mips_blez  :
        case mips_bltz  : return I_RS_C;

        //load upper immediate.
        case mips_lui   : return I_RD_C;

        /********** J type instruction **********/
        case mips_j     :
        case mips_jal   : return J_C;
        case mips_jr    : return J_RS;
        case mips_jalr  : return J_RD_RS;

        default: {
        //Not a identifiable instruction.
        return MIPS_UNKNOWN;
        }
    }
}


/******************************************************************************
* Misc functions.
******************************************************************************/
/* initfunction for the registerName map,   */
static biRegMap initRegisterNameMap() {
    //variable for the register
    biRegMap registerName; 

    registerName.insert(biRegMap::value_type(0,zero));
    registerName.insert(biRegMap::value_type(1,at));    
    registerName.insert(biRegMap::value_type(2,v0));    
    registerName.insert(biRegMap::value_type(3,v1));    

    registerName.insert(biRegMap::value_type(4,a0));    
    registerName.insert(biRegMap::value_type(5,a1));    
    registerName.insert(biRegMap::value_type(6,a2));    
    registerName.insert(biRegMap::value_type(7,a3));    

    registerName.insert(biRegMap::value_type(8,t0));    
    registerName.insert(biRegMap::value_type(9,t1));    
    registerName.insert(biRegMap::value_type(10,t2));    
    registerName.insert(biRegMap::value_type(11,t3));    
    registerName.insert(biRegMap::value_type(12,t4));    
    registerName.insert(biRegMap::value_type(13,t5));    
    registerName.insert(biRegMap::value_type(14,t6));    
    registerName.insert(biRegMap::value_type(15,t7));    

    registerName.insert(biRegMap::value_type(16,s0));    
    registerName.insert(biRegMap::value_type(17,s1));    
    registerName.insert(biRegMap::value_type(18,s2));    
    registerName.insert(biRegMap::value_type(19,s3));    
    registerName.insert(biRegMap::value_type(20,s4));    
    registerName.insert(biRegMap::value_type(21,s5));    
    registerName.insert(biRegMap::value_type(22,s6));    
    registerName.insert(biRegMap::value_type(23,s7));    

    registerName.insert(biRegMap::value_type(24,t8));    
    registerName.insert(biRegMap::value_type(25,t9));    

    registerName.insert(biRegMap::value_type(26,k0));    
    registerName.insert(biRegMap::value_type(27,k1));    

    registerName.insert(biRegMap::value_type(28,gp));    
    registerName.insert(biRegMap::value_type(29,sp));    
    registerName.insert(biRegMap::value_type(30,fp));    
    registerName.insert(biRegMap::value_type(31,ra));    

    return registerName;
}

