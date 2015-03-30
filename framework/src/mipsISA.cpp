/******************************************************************************
* Contains information relevant to instruction which might be needed
* by the framework.
******************************************************************************/

/* header file */
#include "mipsISA.hpp"


/* Return the instruction format. */
ISAtype getInstructionFormat(SgAsmMipsInstruction* inst) {
    //check the variantT.
    switch (inst->variantT) {
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
        case mips_jr    : return J_RD;
        case mips_jalr  : return J_RD_RS;

        default: {
        //Not a identifiable instruction.
        return MIPS_UNKNOWN;
        }
    }
}




