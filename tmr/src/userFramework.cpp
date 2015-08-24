

//Include the users header.
#include "userFramework.hpp"


//constructor.
userFramework::userFramework(int argc, char** argv) {
    //initialize the framework with a binary.
    initialize(argc, argv); 
}


int main(int argc, char** argv) {
    /*  Check that an function name has been supplied as argument. */
    if (4 != argc) {
        std::cout << "Wrong number of arguments." << std::endl
            << "Needed: Binary that is to be transformed." << std::endl
            << "        Function name of function to transform." << std::endl
            << "        Transformation mode (0 or 1), 1 is optimized mode." << std::endl;
        exit(0);
    }
    /*  String variable containing the function name. */
    std::string functionName = std::string(argv[2]);
    int mode = strtol(&argv[3][0], NULL, 0);

    userFramework* ut = new userFramework(argc-2, argv);

    /* set which function is to be transformed */
    ut->functionSelect(functionName);
    /* enable printing */
    ut->setDebug(true);
    /* use optimized transform, if selected. */
    if (1 == mode) {
        ut->useOptimizedTransform();
    }
    /* transform the function */
    ut->transformBinary();

    return 0;
}

/*  Function that applies tmr   */
void userFramework::arithmeticTMR() {
    /* This function should be able to apply common tmr to many functions */
    /* Save the original instruction */
    saveInstruction();
    /*  Depending on the instruction kind i select what kind
        the TMR instructions should be. */
    MipsInstructionKind duplicateKind;
    std::string tmrMnemonic;

    switch(currentInst.kind) {
        case mips_add:
        case mips_addu:
        case mips_sub:
        case mips_subu:{
            duplicateKind = mips_addu;
            tmrMnemonic = "addu";
            break;
        }
        case mips_mul:{
            duplicateKind = mips_mul;
            tmrMnemonic = "mul";
            break;
        }
        //TODO case if i have a immediate instruction instead. can i solve that?
        case mips_addi:
        case mips_addiu:{
            duplicateKind = mips_addiu;
            tmrMnemonic = "addiu";
            break;
        }
    }
    /* Add two new add instructions using the original input operands */
    instructionStruct firstDup;
    instructionStruct secondDup;

    /* Transfer relevant information to the duplicated instructions  */
    firstDup.kind = duplicateKind;
    firstDup.mnemonic = tmrMnemonic;
    firstDup.format = getInstructionFormat(duplicateKind);
    firstDup.sourceRegisters = currentInst.sourceRegisters;
    //Destination will be different.
    firstDup.instructionConstant = currentInst.instructionConstant;
    firstDup.significantBits = currentInst.significantBits;
    firstDup.memoryReferenceSize = currentInst.memoryReferenceSize;
    firstDup.isSignedMemory = currentInst.isSignedMemory;
    //the address is not copied.
    /* Transfer relevant information to the duplicated instructions  */
    secondDup.kind = duplicateKind;
    secondDup.mnemonic = tmrMnemonic;
    secondDup.format = getInstructionFormat(duplicateKind);
    secondDup.sourceRegisters = currentInst.sourceRegisters;
    //Destination will be different.
    secondDup.instructionConstant = currentInst.instructionConstant;
    secondDup.significantBits = currentInst.significantBits;
    secondDup.memoryReferenceSize = currentInst.memoryReferenceSize;
    secondDup.isSignedMemory = currentInst.isSignedMemory;
    //the address is not copied.
    /* Generate symbolic destination registers and add them */
    registerStruct regOne = generateSymbolicRegister();
    registerStruct regTwo = generateSymbolicRegister();
    
    firstDup.destinationRegisters.push_back(regOne);
    secondDup.destinationRegisters.push_back(regTwo);
    /* Build instructions and insert them */
    SgAsmMipsInstruction* firstMipsDup = buildInstruction(&firstDup);
    SgAsmMipsInstruction* secondMipsDup = buildInstruction(&secondDup);

    /* insert the duplicated instructions. */
    insertInstruction(firstMipsDup);
    //std::cout << "First insertion" << std::endl;
    insertInstruction(secondMipsDup);
    //std::cout << "Second insertion" << std::endl;

    /*  Another add to combine the result of two inserted adds,
        one more to combine the result of the original instruction
        result and the combined result. */
    instructionStruct accumulationOne;
    accumulationOne.kind = mips_addu;
    accumulationOne.mnemonic = "addu";
    accumulationOne.format = getInstructionFormat(mips_addu);
    /* Set the source registers from duplicated instructions as source */
    accumulationOne.sourceRegisters.push_back(regOne);
    accumulationOne.sourceRegisters.push_back(regTwo);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct regAcc = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    accumulationOne.destinationRegisters.push_back(regAcc);
    SgAsmMipsInstruction* accInst = buildInstruction(&accumulationOne);
    insertInstruction(accInst);
    //std::cout << "Third insertion" << std::endl;
    
    /* use one of the generated register and set it to three
        so it can be used in the division. */
    instructionStruct denomInst;
    denomInst.kind = mips_addiu;
    denomInst.mnemonic = "addiu";
    denomInst.format = getInstructionFormat(mips_addiu);
    denomInst.instructionConstant = 0x3;
    denomInst.significantBits = 32;
    /*  Generate a register to store the constant. */
    registerStruct constReg = generateSymbolicRegister();
    /* Set the source register, needs to be zero */
    registerStruct zeroReg;
    zeroReg.regName = zero;
    denomInst.sourceRegisters.push_back(zeroReg);
    denomInst.destinationRegisters.push_back(constReg);
    SgAsmMipsInstruction* divConst = buildInstruction(&denomInst);
    insertInstruction(divConst);
    //std::cout << "Fourth insertion" << std::endl;

    /* final accumulation instruction */
    instructionStruct accumulationFinal;
    accumulationFinal.kind = mips_addu;
    accumulationFinal.mnemonic = "addu";
    accumulationFinal.format = getInstructionFormat(mips_addu);
    /* Set the source registers, one symbolic. */
    accumulationFinal.sourceRegisters.push_back(regAcc);
    registerStruct orgInstDest = currentInst.destinationRegisters.back();
    accumulationFinal.sourceRegisters.push_back(orgInstDest);
    /* set destination register, its the RD from the original instruction. */
    accumulationFinal.destinationRegisters.push_back(orgInstDest);
    /* build and insert instruction */
    SgAsmMipsInstruction* accFinal = buildInstruction(&accumulationFinal);
    insertInstruction(accFinal);
    //std::cout << "Firth insertion" << std::endl;

    /* division instruction with the total sum of the adds
        that is divided by three. Giving us the average value.  */
    instructionStruct divInst;
    divInst.kind = mips_divu;
    divInst.mnemonic = "divu";
    divInst.format = getInstructionFormat(mips_divu);
    /* set the operands of the division, nominator then denominator */
    divInst.sourceRegisters.push_back(constReg);
    divInst.sourceRegisters.push_back(orgInstDest);
    /* Build the instruction and insert it*/
    SgAsmMipsInstruction* divisionInst = buildInstruction(&divInst);
    insertInstruction(divisionInst);
    //std::cout << "Sixth insertion" << std::endl;

    /* Move the result from the special register low */
    instructionStruct moveInst;
    moveInst.kind = mips_mflo;
    moveInst.mnemonic = "mflo";
    moveInst.format = getInstructionFormat(mips_mflo);
    /* set the original destination register as destination */
    moveInst.destinationRegisters.push_back(orgInstDest);
    /* build and insert instruction */
    SgAsmMipsInstruction* move = buildInstruction(&moveInst);
    insertInstruction(move);
    //std::cout << "Seventh insertion" << std::endl;
}

/*  Create TMR for condition testing instructions.
    It also handles logical instructions.
    It does TMR with OR and AND instructions.
    The expression can be abstracted as
    (a AND b) OR (b AND c) OR (c AND a). */
void userFramework::conditionAndLogicalTMR() {
    /* This function should be able to apply common tmr to many functions */
    /* Save the original instruction */
    saveInstruction();
    /*  Depending on the instruction kind i select what kind
        the TMR instructions should be. */
    MipsInstructionKind duplicateKind = currentInst.kind;
    std::string tmrMnemonic = currentInst.mnemonic;


    /* Add two new add instructions using the original input operands */
    instructionStruct firstDup;
    instructionStruct secondDup;

    /* Transfer relevant information to the duplicated instructions  */
    firstDup.kind = duplicateKind;
    firstDup.mnemonic = tmrMnemonic;
    firstDup.format = getInstructionFormat(duplicateKind);
    firstDup.sourceRegisters = currentInst.sourceRegisters;
    //Destination will be different.
    firstDup.instructionConstant = currentInst.instructionConstant;
    firstDup.significantBits = currentInst.significantBits;
    firstDup.memoryReferenceSize = currentInst.memoryReferenceSize;
    firstDup.isSignedMemory = currentInst.isSignedMemory;
    //the address is not copied.
    /* Transfer relevant information to the duplicated instructions  */
    secondDup.kind = duplicateKind;
    secondDup.mnemonic = tmrMnemonic;
    secondDup.format = getInstructionFormat(duplicateKind);
    secondDup.sourceRegisters = currentInst.sourceRegisters;
    //Destination will be different.
    secondDup.instructionConstant = currentInst.instructionConstant;
    secondDup.significantBits = currentInst.significantBits;
    secondDup.memoryReferenceSize = currentInst.memoryReferenceSize;
    secondDup.isSignedMemory = currentInst.isSignedMemory;
    //the address is not copied.
    /* Generate symbolic destination registers and add them */
    registerStruct regOne = generateSymbolicRegister();
    registerStruct regTwo = generateSymbolicRegister();
    
    firstDup.destinationRegisters.push_back(regOne);
    secondDup.destinationRegisters.push_back(regTwo);
    /* Build instructions and insert them */
    SgAsmMipsInstruction* firstMipsDup = buildInstruction(&firstDup);
    SgAsmMipsInstruction* secondMipsDup = buildInstruction(&secondDup);

    /* insert the duplicated instructions. */
    insertInstruction(firstMipsDup);
    //std::cout << "First insertion" << std::endl;
    insertInstruction(secondMipsDup);
    //std::cout << "Second insertion" << std::endl;

    /*  AND the first and second instructions result, original & regOne (1) */
    instructionStruct firstAND;
    firstAND.kind = mips_and;
    firstAND.mnemonic = "and";
    firstAND.format = getInstructionFormat(mips_and);
    /* Set the source registers from duplicated instructions as source */
    firstAND.sourceRegisters = currentInst.destinationRegisters;
    firstAND.sourceRegisters.push_back(regOne);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andResOne = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    firstAND.destinationRegisters.push_back(andResOne);
    SgAsmMipsInstruction* andOne= buildInstruction(&firstAND);
    insertInstruction(andOne);

    /*  AND the second and third instructions result, regOne & regTwo (2) */
    instructionStruct secondAND;
    secondAND.kind = mips_and;
    secondAND.mnemonic = "and";
    secondAND.format = getInstructionFormat(mips_and);
    /* Set the source registers from duplicated instructions as source */
    secondAND.sourceRegisters.push_back(regOne);
    secondAND.sourceRegisters.push_back(regTwo);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andResTwo = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    secondAND.destinationRegisters.push_back(andResTwo);
    SgAsmMipsInstruction* andTwo = buildInstruction(&secondAND);
    insertInstruction(andTwo);

    /*  AND the thirds and first instructions result, regTwo & original (3) */
    instructionStruct thirdAND;
    thirdAND.kind = mips_and;
    thirdAND.mnemonic = "and";
    thirdAND.format = getInstructionFormat(mips_and);
    /*  Set the source registers. */
    thirdAND.sourceRegisters = currentInst.destinationRegisters;
    thirdAND.sourceRegisters.push_back(regTwo);
    /*  Create a destination registers. */
    registerStruct andResThird = generateSymbolicRegister();
    /*  Set the destination register. */
    thirdAND.destinationRegisters.push_back(andResThird);
    SgAsmMipsInstruction* andThree = buildInstruction(&thirdAND);
    insertInstruction(andThree);

    /*  OR the results of (1) and (2). (4) */
    instructionStruct firstOR;
    firstOR.kind = mips_or;
    firstOR.mnemonic = "or";
    firstOR.format = getInstructionFormat(mips_or);
    /*  Set the source registers, result of (1) and (2). */
    firstOR.sourceRegisters.push_back(andResOne);
    firstOR.sourceRegisters.push_back(andResTwo);
    /*  Create a destination register. */
    registerStruct orResOne = generateSymbolicRegister();
    /*  Set the destination registers. */
    firstOR.destinationRegisters.push_back(orResOne);
    SgAsmMipsInstruction* orFirst = buildInstruction(&firstOR);
    insertInstruction(orFirst);

    /*  OR the result of (3) and (4). (5). Put this result in the original register. */
    instructionStruct secondOR;
    secondOR.kind = mips_or;
    secondOR.mnemonic = "or";
    secondOR.format = getInstructionFormat(mips_or);
    /*  Set source registers, destination regs of (3) and (4). */
    secondOR.sourceRegisters.push_back(andResThird);
    secondOR.sourceRegisters.push_back(orResOne);
    /*  Create a destination register. */
    registerStruct orResTwo = generateSymbolicRegister();
    /*  Set the destination register. */
    secondOR.destinationRegisters = currentInst.destinationRegisters;
    //secondOR.destinationRegisters.push_back(orResTwo);
    SgAsmMipsInstruction* orSecond = buildInstruction(&secondOR);
    insertInstruction(orSecond);

}

/* The user defined decision function */
void userFramework::transformDecision(SgAsmMipsInstruction* inst) {
    /* Decode the instruction to get the information  */
    currentInst = decodeInstruction(inst);
    /* Depending on the kind of instruction do appropriate transformation
        consider more functions. */
    switch(currentInst.kind) {
        case mips_add:
        case mips_addu:
        case mips_sub:
        case mips_subu:
        case mips_mul:
        case mips_addi:
        case mips_addiu:{
        //TODO im missing div, divu, mult, multu, they can maybe be their on instruction.
            arithmeticTMR();
            break;
        }
        /*  Condition instructions. */
        case mips_slt:
        case mips_sltu:
        case mips_slti:
        case mips_sltiu:

        /*  Left shift instructions. */
        case mips_sll:
        case mips_sllv:
        /*  Right shift instructions, arithmetic. */
        case mips_sra:
        case mips_srav:
        /*  Right shift instructions, logical. */
        case mips_srl:
        case mips_srlv:

        /*  Logical and bit operations. */
        case mips_and:
        case mips_andi:
        case mips_or:
        case mips_ori:
        case mips_xor:
        case mips_xori:
        case mips_nor:
            /*  Call TMR function. */
            conditionAndLogicalTMR();
            break;
        default: {
            saveInstruction();
        }
    }
}


