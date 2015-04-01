/******************************************************************************
* Contains information relevant to instruction which might be needed
* by the framework. Handles decoding of instructions
******************************************************************************/

/* header file */
#include "mipsISA.hpp"

/* function forward declarations */
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
            decodeInstructionR(format, inst);
            break;
        }
        case I_RD_RS_C  :
        case I_RD_C     :
        case I_RS_RT_C  :
        case I_RS_C     : {
            /* Decode I type instructions.  */
            decodeInstructionI(format, inst);
            break;
        }
        case J_C        :
        case J_RS       :
        case J_RD_RS    : {
            /* Decode J type instructions.  */
            decodeInstructionJ(format, inst);
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
    instructionStruct info;
    /* get the operand list */
    
    
    /* Decode according to the format, save relevant data in the struct.  */
    switch (format) {
        case R_RD_RS_RT :{
             
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
        case R_NOP      : {
            //Nop has nothing here. need to verify... need to verify...
        }
    }
    //returning the filled struct.
    return info;
} 

/* Decode I type instructions.  */
instructionStruct decodeInstructionI(instructionType format, SgAsmMipsInstruction* inst) {
    /* variable for struct */
    instructionStruct info;
    /* decode according to format */
    switch (format) {
        case I_RD_RS_C   :
        case I_RD_C      :
        case I_RS_RT_C   :
        case I_RS_C      : {
            break;
        }
    }
    //returning the filled struct.
    return info;
}

/* Decode J type instructions.  */
instructionStruct decodeInstructionJ(instructionType format, SgAsmMipsInstruction* inst) {
    /* variable for struct */
    instructionStruct info;
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
    return info;
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
        //register found.
        
    }
    
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


