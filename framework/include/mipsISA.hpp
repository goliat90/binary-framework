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
    //decode R args (R1, R2, R3, const)
    R_RD_RS_RT,     //add, addu, and, mul, nor, or, slt, sltu, sub, subu, xor, sllv, srav, srlv
    R_RD_RS_C,      //sll, sra, srl,
    R_RS_RT,        //div, divu, madd, maddu, msub, msubu, mult, multu, 
    R_RD,           //mflo, mfhi,
    R_RS,           //mthi, mtlo,
    R_NOP,          //nop

    //decode I args (const, reg1, reg2(default == null))
    I_RD_RS_C,      //addi, addiu, andi, ori, xori, slti, sltiu, lb, lbu, lh, lhu, lw, lwl, lwr,
    I_RS_RT_C,      //beq, bne, sb, sh, sw, swl, swr,
    I_RS_C,         //bgez, bgezal, bgtz, blez, bltz, 
    I_RD_C,         //lui, 

    //decode J args (
    J_C,            //j(jump), jal,
    J_RS,           //jr,
    J_RD_RS,        //jalr,

    MIPS_UNKNOWN    //The instruction is not included and format is therefore unknown. 
};

// -------- instruction struct --------
// Contains information that is useful for the framework about
// the current instruction.
struct instructionInformation {
    //nmemonic enum.
    MipsInstructionKind kind;
    //nmemonic string.
    std::string mnemonic;
    //input register(s)
    std::vector<std::string> inregisters;
    //output registers
    std::vector<std::string> outregisters;
    //operands list
    //SgAsmOperandList* operandsList;
    //operands list, but as SgAsmExpressionPtrList, one level lower than previous.
    //SgAsmExpressionPtrList* operandExpressionList;
    //if the instruction uses a constant then save it and significant bits.
    uint64_t instructionConstant;
    size_t significantBits;
    //The number of bits/size of the memory reference, 8,16,32,64
    int memoryReferenceSize;
    //address of the instruction, if the instruction is inserted then it
    //is a temporary value or invalid.
    rose_addr_t address;
};


#endif
