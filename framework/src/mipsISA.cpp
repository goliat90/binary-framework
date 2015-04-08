/******************************************************************************
* Contains information relevant to instruction which might be needed
* by the framework. Handles decoding of instructions
******************************************************************************/

/* header file */
#include "mipsISA.hpp"

/* forward function declarations */
/* Format of the instruction */
instructionType getInstructionFormat(SgAsmMipsInstruction*);
/* decode register names */
registerStruct decodeRegister(SgAsmExpression*); 
/* decode value expression, a constant */
void decodeValueExpression(SgAsmExpression*, instructionStruct*);
/* decode the memory expression */
void decodeMemoryReference(SgAsmExpression*, instructionStruct*);
/* operandList decoder, decodes the operands in the list. */
instructionStruct decodeOpList(SgAsmExpressionPtrList*, bool, bool, bool, bool, bool);
/* initfunction for the registerName map,   */
static std::map<unsigned, mipsRegisterName> initRegisterNameMap();

/* Global variables */
/* map for the register names. Mappes values of registers to the enum name.  */
static std::map<unsigned, mipsRegisterName> registerNameMap = initRegisterNameMap(); 


/* decode instruction. Calls on the R,I or J decode functions. */
instructionStruct decodeinstruction(SgAsmMipsInstruction* inst) {
    instructionStruct instStruct;
    /* Check what kind of instruction format it is.  */
    instructionType format = getInstructionFormat(inst);
    /* get the operand list */
    SgAsmExpressionPtrList* operandList = &inst->get_operandList()->get_operands();
    /* depending on instruction format use right arguments for decode function */
    switch (format) {
        /* R instructions */
        case R_RD_RS_RT :
            instStruct = decodeOpList(operandList, true, true, true, false, false);
            break;
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
        case R_NOP      : {
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
    instStruct.format = format;
    instStruct.address = inst->get_address();

    /* Return the insturction struct from the decoding */    
    return instStruct;
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

/* decode a register operand */
registerStruct decodeRegister(SgAsmExpression* expr) {
    /* the register struct */
    registerStruct regStruct;
    //cast it to register expression
    SgAsmDirectRegisterExpression* regExpr = isSgAsmDirectRegisterExpression(expr);
    //get the register descriptor, from it i can get majr(number)
    RegisterDescriptor reg = regExpr->get_descriptor();
    //check if the register exists in the namemap
    if (registerNameMap.find(reg.get_major()) != registerNameMap.end()) {
        //register found, get the iterator and get the register enum.
        regStruct.regName = registerNameMap.find(reg.get_major())->second;
        /* If the register is zero then check if it is symbolic. */
        if (true == isSymbolicRegister(regExpr)) {
            /* Set the regName member to the symbolic enum */
            regStruct.regName = symbolic_reg;
            /* Save the symbolic register number. */
            regStruct.symbolicNumber = findSymbolicRegister(regExpr);
        }
    } else {
        //the register is not an existing one.
        regStruct.regName = reg_fault;
    }
    return regStruct;
}

/* decode value expression, a constant. Save values in the struct */
void decodeValueExpression(SgAsmExpression* inst, instructionStruct* instStruct) {
    /* cast the SgAsmExpression */
    SgAsmIntegerValueExpression* ve = isSgAsmIntegerValueExpression(inst); 
    /* save the constant  */
    instStruct->instructionConstant = ve->get_absoluteValue();
    /* save the significant bits, needed? */
    instStruct->significantBits = ve->get_significantBits();
}

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

/* Return the instruction format. Possibly change this to decode instruction.*/
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


/* initfunction for the registerName map,   */
static std::map<unsigned, mipsRegisterName> initRegisterNameMap() {
    //variable for the register
    std::map<unsigned, mipsRegisterName> registerName; 

    registerName.insert(std::pair<unsigned, mipsRegisterName>(0,zero));
    registerName.insert(std::pair<unsigned, mipsRegisterName>(1,at));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(2,v0));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(3,v1));    

    registerName.insert(std::pair<unsigned, mipsRegisterName>(4,a0));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(5,a1));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(6,a2));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(7,a3));    

    registerName.insert(std::pair<unsigned, mipsRegisterName>(8,t0));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(9,t1));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(10,t2));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(11,t3));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(12,t4));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(13,t5));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(14,t6));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(15,t7));    

    registerName.insert(std::pair<unsigned, mipsRegisterName>(16,s0));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(17,s1));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(18,s2));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(19,s3));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(20,s4));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(21,s5));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(22,s6));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(23,s7));    

    registerName.insert(std::pair<unsigned, mipsRegisterName>(24,t8));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(25,t9));    

    registerName.insert(std::pair<unsigned, mipsRegisterName>(26,k0));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(27,k1));    

    registerName.insert(std::pair<unsigned, mipsRegisterName>(28,gp));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(29,sp));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(30,fp));    
    registerName.insert(std::pair<unsigned, mipsRegisterName>(31,ra));    

    return registerName;
}

