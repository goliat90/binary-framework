/******************************************************************************
* Contains information relevant to instruction which might be needed
* by the framework.
******************************************************************************/

/* header file */
#include "mipsISA.hpp"


/* Identify the instruction and unparse the operands. */
ISAtype getInstructionFormat(SgAsmMipsInstruction* inst) {
    //check the variantT.
    switch (inst->variantT) {
        //arithmetic.
        case mips_add   :
        case mips_addu  :
        case mips_sub   :
        case mips_subu  :
        case mips_addi  :
        case mips_addiu :

        //multiply and div
        case mips_mul   :
        case mips_div   :
        case mips_divu  :
        case mips_madd  :
        case mips_maddu :
        case mips_msub  :
        case mips_msubu :
        case mips_mult  :
        case mips_multu :

        //logical ops.
        case mips_and   :   
        case mips_nor   :   
        case mips_or    :   
        case mips_xor   :   
        case mips_andi  :
        case mips_ori   :   
        case mips_xori  :

        //Shift ops.
        case mips_sllv  :
        case mips_srav  :
        case mips_srlv  :
        case mips_sll   :   
        case mips_sra   :   
        case mips_srl   :   

        //condition, testing
        case mips_slt   :
        case mips_sltu  :
        case mips_slti  :
        case mips_sltiu :

        //accumulator ops.
        case mips_mflo  :
        case mips_mfhi  :
        case mips_mthi  :
        case mips_mtlo  :

        //conditional branch and jumps
        case mips_beq   :
        case mips_bne   :
        case mips_bgez  :
        case mips_bgezal:
        case mips_bgtz  :
        case mips_blez  :
        case mips_bltz  :
        case mips_j     :
        case mips_jal   :
        case mips_jr    :
        case mips_jalr  :

        //load and store.
        case mips_lb    :
        case mips_lbu   :
        case mips_lh    :
        case mips_lhu   :
        case mips_lw    :
        case mips_lwl   :
        case mips_lwr   :
        case mips_sb    :
        case mips_sh    :
        case mips_sw    :
        case mips_swl   :
        case mips_swr   :
        case mips_lui   :

        case mips_nop   :
        default: {
        //Not a identifiable instruction.
        }
    }
}


