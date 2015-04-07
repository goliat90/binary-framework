/******************************************************************************
* Contains information relevant to instruction which might be needed
* by the framework. Handles decoding of instructions
******************************************************************************/

/* header file */
#include "mipsISA.hpp"

/* forward function declarations */
/* Format of the instruction */
instructionType     getInstructionFormat(SgAsmMipsInstruction*);
/* Decode the instruction, calls on the specific decode instructions. */ 
instructionStruct   decodeInstruction(SgAsmMipsInstruction*);
/* Decode R type instructions.  */
instructionStruct decodeInstructionR(instructionType format, SgAsmMipsInstruction*);
/* Decode I type instructions.  */
instructionStruct decodeInstructionI(instructionType format, SgAsmMipsInstruction*);
/* Decode J type instructions.  */
instructionStruct decodeInstructionJ(instructionType format, SgAsmMipsInstruction*);
/* decode register names */
mipsRegisterName decodeRegister(SgAsmExpression*); 
/* decode value expression, a constant */
void decodeValueExpression(SgAsmExpression*, instructionStruct*);
/* decode the memory expression */
void decodeMemoryReference(SgAsmExpression*, instructionStruct*);
/* operandList decoder, decodes the operands in the list. */
instructionStruct decodeOpList(SgAsmExpressionPtrList*, bool, bool, bool, bool, bool);

/* initfunction for the registerName map,   */
static std::map<unsigned, mipsRegisterName> initRegisterNameMap();

/* map for the register names. Mappes values of registers to the enum name.  */
static std::map<unsigned, mipsRegisterName> registerNameMap = initRegisterNameMap(); 

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

/* decode instruction. Calls on the R,I or J decode functions. */
instructionStruct decodeinstruction(SgAsmMipsInstruction* inst) {
    instructionStruct instructionInfo;
    /* Check what kind of instruction format it is.  */
    instructionType format = getInstructionFormat(inst);
    /* depending on instruction format use appropriate decode function */
    switch (format) {
        case R_RD_RS_RT :
        case R_RD_RS_C  :
        case R_RD       :
        case R_RS_RT    :
        case R_RS       :
        case R_NOP      : {
            instructionInfo = decodeInstructionR(format, inst);
            break;
        }
        case I_RD_RS_C  :
        case I_RD_C     :
        case I_RS_RT_C  :
        case I_RS_C     : {
            /* Decode I type instructions.  */
            instructionInfo = decodeInstructionI(format, inst);
            break;
        }
        case J_C        :
        case J_RS       :
        case J_RD_RS    : {
            /* Decode J type instructions.  */
            instructionInfo = decodeInstructionJ(format, inst);
            break;
        }
        default: {
            //The instruction is unknown.
            
        }
    }
    /* Save the address of the instruction, consider other
       values that are common to all instructions. kind,mnemonic, */
    instructionInfo.kind = inst->get_kind();
    instructionInfo.mnemonic = inst->get_mnemonic();
    instructionInfo.format = format;
    instructionInfo.address = inst->get_address();

    /* Return the insturction struct from the decoding */    
    return instructionInfo;
}

/* Decode R type instruction specific things..  */
instructionStruct decodeInstructionR(instructionType format, SgAsmMipsInstruction* inst) {
    /* variable for struct */
    instructionStruct instStruct;
    /* get the operand list */
    SgAsmExpressionPtrList* operandList = &inst->get_operandList()->get_operands();
    /* Decode according to the format, save relevant data in the struct.  */
    switch (format) {
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
        case R_NOP      : {
        }
    }
    //returning the filled struct.
    return instStruct;
} 

/* Decode I type instructions.  */
instructionStruct decodeInstructionI(instructionType format, SgAsmMipsInstruction* inst) {
    /* variable for struct */
    instructionStruct instStruct;
    /* get the operand list */
    SgAsmExpressionPtrList* operandList = &inst->get_operandList()->get_operands();
    /* decode according to format */
    switch (format) {
        case I_RD_RS_C   :
            instStruct = decodeOpList(operandList, true, true, false, true, false);
            break;
        case I_RD_C      :
        case I_RS_RT_C   :
        case I_RS_C      : {
            break;
        }
    }
    //returning the filled struct.
    return instStruct;
}

/* Decode J type instructions.  */
instructionStruct decodeInstructionJ(instructionType format, SgAsmMipsInstruction* inst) {
    /* variable for struct */
    instructionStruct instStruct;
    /* decode instruction with the right format */
    switch (format) {
        case J_C        :
        case J_RS       :
        case J_RD_RS    : {
            //J instruction decoding.
            break;
        }
    }
    //returning the filled struct.
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
        mipsRegisterName RDname = decodeRegister((*operandList)[opIndex]);       
        /* Insert the register into the struct as a destination register */
        instruction.destinationRegisters.push_back(RDname);
        /* Increment the operand index */
        opIndex++;
    }
    if (true == hasRS) {
        /* Has a rs register operand, extract it. */
        mipsRegisterName RSname = decodeRegister((*operandList)[opIndex]);       
        /* insert the registers into the struct as a source register */
        instruction.sourceRegisters.push_back(RSname);
        /* Increment the operand index */
        opIndex++;
    }
    if (true == hasRT) {
        /* Has a rt register operand, extract it. */
        mipsRegisterName RTname = decodeRegister((*operandList)[opIndex]);       
        /* insert the registers into the struct as a source register */
        instruction.sourceRegisters.push_back(RTname);
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
mipsRegisterName decodeRegister(SgAsmExpression* expr) {
    //variable for the registername
    mipsRegisterName regName;
    //cast it to register expression
    SgAsmDirectRegisterExpression* regExpr = isSgAsmDirectRegisterExpression(expr);
    //get the register descriptor, from it i can get majr(number)
    RegisterDescriptor reg = regExpr->get_descriptor();
    //check if the register exists in the namemap
    if (registerNameMap.find(reg.get_major()) != registerNameMap.end()) {
        //register found, get the iterator and get the register enum.
        regName = registerNameMap.find(reg.get_major())->second;
    } else {
        //the register was not found
        regName = reg_fault;
    }
    return regName;
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
    mipsRegisterName regName = decodeRegister(binExp->get_lhs());
    instStruct->sourceRegisters.push_back(regName);
    /* decode the constant and pass the struct as well */
    decodeValueExpression(binExp->get_rhs(), instStruct);
}


/* Return the instruction format. Possibly change this to decode instruction.*/
//change the return type.
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
        case mips_sltiu :

        //load operations.
        case mips_lb    :
        case mips_lbu   :
        case mips_lh    :
        case mips_lhu   :
        case mips_lw    :
        case mips_lwl   :
        case mips_lwr   : return I_RD_RS_C;

        //conditional branch 
        case mips_beq   :
        case mips_bne   :

        //store operations
        case mips_sb    :
        case mips_sh    :
        case mips_sw    :
        case mips_swl   :
        case mips_swr   : return I_RS_RT_C;

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


