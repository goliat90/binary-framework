/* Header file for InstructionInformation */

#ifndef MIPSISA_H
#define MIPSISA_H

/* Headers */
#include "rose.h"
#include "symbolicRegisters.hpp"
#include "boost/bimap.hpp"

/* forward declarations */
struct instructionStruct;

/*  Typedefs. */
typedef std::vector<registerStruct> regStructVector;

//Type of instruction syntax
//RD = Destiniation register
//RS,RT = Source operand registers
enum instructionType {
    //decode R 
    R_RD_RS_RT,     //add, addu, and, mul, nor, or, slt, sltu, sub, subu, xor, sllv, srav, srlv, movn
    R_RD_RS_C,      //sll, sra, srl,
    R_RD,           //mflo, mfhi,
    R_RS_RT,        //div, divu, madd, maddu, msub, msubu, mult, multu, 
    R_RS,           //mthi, mtlo,
    R_NOP,          //nop

    //decode I 
    I_RD_RS_C,      //addi, addiu, andi, ori, xori, slti, sltiu, 
    I_RD_MEM_RS_C,  //lb, lbu, lh, lhu, lw, lwl, lwr, (instruction with a combined operand of register and constant)
    I_RD_C,         //lui, 
    I_RS_RT_C,      //beq, bne, 
    I_RS_MEM_RT_C,  //sb, sh, sw, swl, swr,(instruction with a combined operand of register and constant)
    I_RS_C,         //bgez, bgezal, bgtz, blez, bltz, 

    //decode J 
    J_C,            //j(jump), jal,
    J_RS,           //jr,
    J_RD_RS,        //jalr,

    MIPS_UNKNOWN    //The instruction is not included and format is therefore unknown. 
};

//function declarations.
/* Decode the instruction, calls on the specific decode instructions. */
instructionStruct decodeInstruction(SgAsmMipsInstruction*);
/* Builds an instruction from an instructionStruct */
SgAsmMipsInstruction* buildInstruction(instructionStruct*);
/* Return the format of an instruction defined by the framework */
instructionType getInstructionFormat(MipsInstructionKind);
/* decode a register operand */
registerStruct decodeRegister(SgAsmExpression*);
/* Creates a register expression */
SgAsmDirectRegisterExpression* buildRegister(registerStruct);

// -------- instruction struct --------
// Contains information that is useful for the framework about
// the current instruction.
struct instructionStruct {
    //Constructor
    instructionStruct():kind(mips_unknown_instruction), mnemonic(""), format(MIPS_UNKNOWN),
    instructionConstant(0), significantBits(0), memoryReferenceSize(0), isSignedMemory(false), address(0){};
    //nmemonic enum.
    MipsInstructionKind kind;
    //mnemonic string
    std::string mnemonic;
    //instruction format.
    instructionType format;
    //input register(s)
    std::vector<registerStruct> destinationRegisters;
    //output registers
    std::vector<registerStruct> sourceRegisters;

    //if the instruction uses a constant then save it and significant bits.
    //The constant value
    uint64_t instructionConstant;
    //How many of the bits in the constant value are significant
    int significantBits;
    //if the value is signed.
    bool isSignedConstant;
   
    /* Information related to information */ 
    //TODO consider adding segment to saved information.
    //The number of bits/size of the memory reference, 8,16,32,64
    int memoryReferenceSize;
    //is the memory value signed.
    bool isSignedMemory;
    
    //It is relevant to memory expressions.
    //address of the instruction, if the instruction is inserted then it is a temporary value.
    rose_addr_t address; 
};
#endif

