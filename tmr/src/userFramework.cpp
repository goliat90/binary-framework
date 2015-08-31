

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
    /* enable printing */
    ut->setDebug(true);

    /* set which function is to be transformed */
    ut->functionSelect(functionName);
    /* use optimized transform, if selected. */
    if (1 == mode) {
        ut->useOptimizedTransform();
    }
    /* transform the function */
    ut->transformBinary();

    return 0;
}


/*  Create median TMR instructions.
    It also handles logical instructions.
    It does TMR with OR and AND instructions.
    The expression can be abstracted as
    (a AND b) OR (b AND c) OR (c AND a). */
void userFramework::medianTMR() {
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

/*  Median TMR function for instructions that write their result to the
    acc register. It performs it the same way as the medianTMR function.
    The difference is that it places the TMR result back into the acc register.
    The median TMR is also performed seperatley on high and low. */
void userFramework::accumulatorMedianTMR() {
    /* Save the original instruction */
    saveInstruction();
    /*  Depending on the instruction kind i select what kind
        the TMR instructions should be. */
    MipsInstructionKind duplicateKind = currentInst.kind;
    std::string tmrMnemonic = currentInst.mnemonic;

    /*  Move the results of the original instruction. */
    instructionStruct mfloA0;
    mfloA0.kind = mips_mflo;
    mfloA0.mnemonic = "mflo";
    mfloA0.format = getInstructionFormat(mips_mflo);
    /*  Get generate a regular register for the mfhi. */
    registerStruct mfloRegA0 = generateSymbolicRegister();
    mfloA0.destinationRegisters.push_back(mfloRegA0);
    /* Build instruction and insert it. */
    SgAsmMipsInstruction* mipsMfloA0 = buildInstruction(&mfloA0);
    insertInstruction(mipsMfloA0);

    /* mfhi instruction. */
    instructionStruct mfhiA1;
    mfhiA1.kind = mips_mfhi;
    mfhiA1.mnemonic = "mfhi";
    mfhiA1.format = getInstructionFormat(mips_mfhi);
    /*  Get generate a regular register for the mfhi. */
    registerStruct mfhiRegA1 = generateSymbolicRegister();
    mfhiA1.destinationRegisters.push_back(mfhiRegA1);
    /* Build instruction and insert it. */
    SgAsmMipsInstruction* mipsMfhiA1 = buildInstruction(&mfhiA1);
    insertInstruction(mipsMfhiA1);

    /* Duplicate original instruction using the original input operands */
    instructionStruct accFirstDup;
    /* Transfer relevant information to the duplicated instructions  */
    accFirstDup.kind = duplicateKind;
    accFirstDup.mnemonic = tmrMnemonic;
    accFirstDup.format = getInstructionFormat(duplicateKind);
    accFirstDup.sourceRegisters = currentInst.sourceRegisters;
    //Destination will be different.
    accFirstDup.instructionConstant = currentInst.instructionConstant;
    accFirstDup.significantBits = currentInst.significantBits;
    accFirstDup.memoryReferenceSize = currentInst.memoryReferenceSize;
    accFirstDup.isSignedMemory = currentInst.isSignedMemory;
    /*  Build instruction and insert it. */
    SgAsmMipsInstruction* mipsAccDupOne = buildInstruction(&accFirstDup);
    insertInstruction(mipsAccDupOne);

    /*  Insert move instructions to save duplicated instructions result. */
    instructionStruct mfloB0;
    mfloB0.kind = mips_mflo;
    mfloB0.mnemonic = "mflo";
    mfloB0.format = getInstructionFormat(mips_mflo);
    /*  Get generate a regular register for the mfhi. */
    registerStruct mfloRegB0 = generateSymbolicRegister();
    mfloB0.destinationRegisters.push_back(mfloRegB0);
    /* Build instruction and insert it. */
    SgAsmMipsInstruction* mipsMfloB0 = buildInstruction(&mfloB0);
    insertInstruction(mipsMfloB0);

    /* mfhi instruction. */
    instructionStruct mfhiB1;
    mfhiB1.kind = mips_mfhi;
    mfhiB1.mnemonic = "mfhi";
    mfhiB1.format = getInstructionFormat(mips_mfhi);
    /*  Get generate a regular register for the mfhi. */
    registerStruct mfhiRegB1 = generateSymbolicRegister();
    mfhiB1.destinationRegisters.push_back(mfhiRegB1);
    /* Build instruction and insert it. */
    SgAsmMipsInstruction* mipsMfhiB1 = buildInstruction(&mfhiB1);
    insertInstruction(mipsMfhiB1);

    /*  Duplicate the instruction again. */
    instructionStruct accSecondDup;
    /* Transfer relevant information to the duplicated instructions  */
    accSecondDup.kind = duplicateKind;
    accSecondDup.mnemonic = tmrMnemonic;
    accSecondDup.format = getInstructionFormat(duplicateKind);
    accSecondDup.sourceRegisters = currentInst.sourceRegisters;
    //Destination will be different.
    accSecondDup.instructionConstant = currentInst.instructionConstant;
    accSecondDup.significantBits = currentInst.significantBits;
    accSecondDup.memoryReferenceSize = currentInst.memoryReferenceSize;
    accSecondDup.isSignedMemory = currentInst.isSignedMemory;
    /*  Build instruction and insert it. */
    SgAsmMipsInstruction* mipsAccDupTwo = buildInstruction(&accSecondDup);
    insertInstruction(mipsAccDupTwo);

    /*  Insert move instructions to save the results of duplicated register. */
    instructionStruct mfloC0;
    mfloC0.kind = mips_mflo;
    mfloC0.mnemonic = "mflo";
    mfloC0.format = getInstructionFormat(mips_mflo);
    /*  Get generate a regular register for the mfhi. */
    registerStruct mfloRegC0 = generateSymbolicRegister();
    mfloC0.destinationRegisters.push_back(mfloRegC0);
    /* Build instruction and insert it. */
    SgAsmMipsInstruction* mipsMfloC0 = buildInstruction(&mfloC0);
    insertInstruction(mipsMfloC0);

    /* mfhi instruction. */
    instructionStruct mfloC1;
    mfloC1.kind = mips_mfhi;
    mfloC1.mnemonic = "mfhi";
    mfloC1.format = getInstructionFormat(mips_mfhi);
    /*  Get generate a regular register for the mfhi. */
    registerStruct mfhiRegC1 = generateSymbolicRegister();
    mfloC1.destinationRegisters.push_back(mfhiRegC1);
    /* Build instruction and insert it. */
    SgAsmMipsInstruction* mipsMfhiC1 = buildInstruction(&mfloC1);
    insertInstruction(mipsMfhiC1);

    /*  AND the first and second instructions low result. */
    instructionStruct firstLowAnd;
    firstLowAnd.kind = mips_and;
    firstLowAnd.mnemonic = "and";
    firstLowAnd.format = getInstructionFormat(mips_and);
    /* Set the source registers from mflo instructions as source */
    firstLowAnd.sourceRegisters.push_back(mfloRegA0);
    firstLowAnd.sourceRegisters.push_back(mfloRegB0);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andLowResOne = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    firstLowAnd.destinationRegisters.push_back(andLowResOne);
    SgAsmMipsInstruction* andLowOne= buildInstruction(&firstLowAnd);
    insertInstruction(andLowOne);

    /*  AND the first and second instructions high result. */
    instructionStruct firstHighAnd;
    firstHighAnd.kind = mips_and;
    firstHighAnd.mnemonic = "and";
    firstHighAnd.format = getInstructionFormat(mips_and);
    /* Set the source registers from mflo instructions as source */
    firstHighAnd.sourceRegisters.push_back(mfhiRegA1);
    firstHighAnd.sourceRegisters.push_back(mfhiRegB1);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andHighResOne = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    firstHighAnd.destinationRegisters.push_back(andHighResOne);
    SgAsmMipsInstruction* andHighOne = buildInstruction(&firstHighAnd);
    insertInstruction(andHighOne);


    /*  AND the second and third instructions low result. */
    instructionStruct secondLowAnd;
    secondLowAnd.kind = mips_and;
    secondLowAnd.mnemonic = "and";
    secondLowAnd.format = getInstructionFormat(mips_and);
    /* Set the source registers from mflo instructions as source */
    secondLowAnd.sourceRegisters.push_back(mfloRegB0);
    secondLowAnd.sourceRegisters.push_back(mfloRegC0);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andLowResTwo = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    secondLowAnd.destinationRegisters.push_back(andLowResTwo);
    SgAsmMipsInstruction* andLowTwo = buildInstruction(&secondLowAnd);
    insertInstruction(andLowTwo);

    /*  AND the second and third instructions high result. */
    instructionStruct secondHighAnd;
    secondHighAnd.kind = mips_and;
    secondHighAnd.mnemonic = "and";
    secondHighAnd.format = getInstructionFormat(mips_and);
    /* Set the source registers from mflo instructions as source */
    secondHighAnd.sourceRegisters.push_back(mfhiRegB1);
    secondHighAnd.sourceRegisters.push_back(mfhiRegC1);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andHighResTwo = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    secondHighAnd.destinationRegisters.push_back(andHighResTwo);
    SgAsmMipsInstruction* andHighTwo = buildInstruction(&secondHighAnd);
    insertInstruction(andHighTwo);


    /*  And the third and first instructions low result. */
    instructionStruct thirdLowAnd;
    thirdLowAnd.kind = mips_and;
    thirdLowAnd.mnemonic = "and";
    thirdLowAnd.format = getInstructionFormat(mips_and);
    /* Set the source registers from mflo instructions as source */
    thirdLowAnd.sourceRegisters.push_back(mfloRegA0);
    thirdLowAnd.sourceRegisters.push_back(mfloRegC0);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andLowResThird = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    thirdLowAnd.destinationRegisters.push_back(andLowResThird);
    SgAsmMipsInstruction* andLowThree = buildInstruction(&thirdLowAnd);
    insertInstruction(andLowThree);

    /*  And the third and first instructions high result. */
    instructionStruct thirdHighAnd;
    thirdHighAnd.kind = mips_and;
    thirdHighAnd.mnemonic = "and";
    thirdHighAnd.format = getInstructionFormat(mips_and);
    /* Set the source registers from mflo instructions as source */
    thirdHighAnd.sourceRegisters.push_back(mfhiRegA1);
    thirdHighAnd.sourceRegisters.push_back(mfhiRegC1);
    /*  Create a destination register for the accumulated sum.*/
    registerStruct andHighResThird = generateSymbolicRegister();
    /*  Set the regAcc as destination register. */
    thirdHighAnd.destinationRegisters.push_back(andHighResThird);
    SgAsmMipsInstruction* andHighThree = buildInstruction(&thirdHighAnd);
    insertInstruction(andHighThree);

    /* OR the result of the two first low ANDs. */
    instructionStruct firstLowOr;
    firstLowOr.kind = mips_or;
    firstLowOr.mnemonic = "or";
    firstLowOr.format = getInstructionFormat(mips_or);
    /*  Set the source registers, result of (1) and (2). */
    firstLowOr.sourceRegisters.push_back(andLowResOne);
    firstLowOr.sourceRegisters.push_back(andLowResTwo);
    /*  Create a destination register. */
    registerStruct orResLowOne = generateSymbolicRegister();
    /*  Set the destination registers. */
    firstLowOr.destinationRegisters.push_back(orResLowOne);
    SgAsmMipsInstruction* orFirstLow = buildInstruction(&firstLowOr);
    insertInstruction(orFirstLow);

    /*  OR the result of the two first high ANDs. */
    instructionStruct firstHighOr;
    firstHighOr.kind = mips_or;
    firstHighOr.mnemonic = "or";
    firstHighOr.format = getInstructionFormat(mips_or);
    /*  Set the source registers, result of (1) and (2). */
    firstHighOr.sourceRegisters.push_back(andHighResOne);
    firstHighOr.sourceRegisters.push_back(andHighResTwo);
    /*  Create a destination register. */
    registerStruct orResHighOne = generateSymbolicRegister();
    /*  Set the destination registers. */
    firstHighOr.destinationRegisters.push_back(orResHighOne);
    SgAsmMipsInstruction* orFirstHigh = buildInstruction(&firstHighOr);
    insertInstruction(orFirstHigh);

    /*  OR the result of the last low AND and the OR low result. */
    instructionStruct secondLowOr;
    secondLowOr.kind = mips_or;
    secondLowOr.mnemonic = "or";
    secondLowOr.format = getInstructionFormat(mips_or);
    /*  Set the source registers, result of (1) and (2). */
    secondLowOr.sourceRegisters.push_back(andLowResThird);
    secondLowOr.sourceRegisters.push_back(orResLowOne);
    /*  Create a destination register. */
    registerStruct orResLowTwo = generateSymbolicRegister();
    /*  Set the destination registers. */
    secondLowOr.destinationRegisters.push_back(orResLowTwo);
    SgAsmMipsInstruction* orSecondLow = buildInstruction(&secondLowOr);
    insertInstruction(orSecondLow);

    /*  OR the result of the last high AND and the OR high result. */
    instructionStruct secondHighOr;
    secondHighOr.kind = mips_or;
    secondHighOr.mnemonic = "or";
    secondHighOr.format = getInstructionFormat(mips_or);
    /*  Set the source registers, result of (1) and (2). */
    secondHighOr.sourceRegisters.push_back(andHighResThird);
    secondHighOr.sourceRegisters.push_back(orResHighOne);
    /*  Create a destination register. */
    registerStruct orResHighTwo = generateSymbolicRegister();
    /*  Set the destination registers. */
    secondHighOr.destinationRegisters.push_back(orResHighTwo);
    SgAsmMipsInstruction* orSecondHigh = buildInstruction(&secondHighOr);
    insertInstruction(orSecondHigh);

    /*  Move the final results back into the accumulator registers. */
    instructionStruct mtloRes;
    mtloRes.kind = mips_mtlo;
    mtloRes.mnemonic = "mtlo";
    mtloRes.format = getInstructionFormat(mips_mtlo);
    /*  Set the final low TMR result as source. */
    mtloRes.sourceRegisters.push_back(orResLowTwo);
    /*  Build instruction and insert. */
    SgAsmMipsInstruction* mipsMtloRes = buildInstruction(&mtloRes);
    insertInstruction(mipsMtloRes);

    /*  move the high acc result back. */
    instructionStruct mthiRes;
    mthiRes.kind = mips_mthi;
    mthiRes.mnemonic = "mthi";
    mthiRes.format = getInstructionFormat(mips_mthi);
    /*  Set the final low TMR result as source. */
    mthiRes.sourceRegisters.push_back(orResHighTwo);
    /*  Build instruction and insert. */
    SgAsmMipsInstruction* mipsMthiRes = buildInstruction(&mthiRes);
    insertInstruction(mipsMthiRes);

}


/* The user defined decision function */
void userFramework::transformDecision(SgAsmMipsInstruction* inst) {
    /* Decode the instruction to get the information  */
    currentInst = decodeInstruction(inst);
    /* Depending on the kind of instruction do appropriate transformation
        consider more functions. */
    //TODO im missing div, divu, mult, multu, they can maybe be their on instruction.
    //TODO the acc tmr will probably be done by a median tmr applied seperatley to
    //TODO high and lo registers, end result are moved back to acc.
    switch(currentInst.kind) {
        case mips_div:
        case mips_divu:
        case mips_mult:
        case mips_multu: {
            accumulatorMedianTMR();
            break;
        }
        /*  Arithmetic */
        case mips_add:
        case mips_addu:
        case mips_addi:
        case mips_addiu:
        case mips_sub:
        case mips_subu:
        case mips_mul:
        case mips_lui:

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
            medianTMR();
            break;
        default: {
            saveInstruction();
        }
    }
}


