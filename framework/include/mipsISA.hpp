/* Header file for InstructionInformation */

#ifndef MIPSISA_H
#define IMIPSISA_H

/* Headers */
#include "rose.h"

//function declarations.


//name of the registers.
enum registerName {
    zero,           //always zero,
    at,             //assembly temporary
    v0,v1,          //return value from function call.

    a0,a1,a2,a3,    //first four parameters of function call

    t0,t1,t2,t3,    //temporary variables, no need to preserve
    t4,t5,t6,t7,

    s0,s1,s2,s3,    //function variables, must be preserved.
    s4,s5,s6,s7,

    t8,t9,          //more temporary variables
    k0,k1,          //kernel use register, migh change unexpectedly.
    gp,             //global pointer
    sp,             //stack pointer
    fp,             //stack frame pointer or subroutine variable(s8)
    ra              //return address of the last subroutine call.
};

//Type of instruction syntax
//RD = Destiniation register
//RS,RT = Source operand registers
enum ISAtype{
    R_RD_RS_RT,     //add, addu, and, mul, nor, or, slt, sltu, sub, subu, xor, sllv, srav, srlv
    R_RD_RS_C,      //sll, sra, srl,
    R_RS_RT,        //div, divu, madd, maddu, msub, msubu, mult, multu, 
    R_RD,           //mflo, mfhi,
    R_RS,           //mthi, mtlo,

    I_RD_RS_C,      //addi, addiu, andi, ori, xori, slti, sltiu, lb, lbu, lh, lhu, lw, lwl, lwr,
    I_RS_RT_C,      //beq, bne, sb, sh, sw, swl, swr,
    I_RS_C,         //bgez, bgezal, bgtz, blez, bltz, 
    I_RD_C,         //lui, 

    J_C,            //j(jump), jal,
    J_RS,           //jr,
    J_RD_RS,        //jalr,
    R_NOP           //nop
    
};





#endif
