

//Include the users header.
#include "userFramework.hpp"


//constructor.
userFramework::userFramework(int argc, char** argv) {
    //initialize the framework with a binary.
    initialize(argc, argv); 
}


int main(int argc, char** argv) {

    userFramework* uT = new userFramework(argc, argv);

    /* set which function is to be transformed */
    //functionSelect(string);

    uT->transformBinary();

    return 0;
}


/* The user defined decision function */
void userFramework::transformDecision(SgAsmMipsInstruction* inst) {
    /* Decode the instruction to get the information  */
    instructionStruct currentInst = decodeInstruction(inst);
    /* Depending on the kind of instruction do appropriate transformation
        consider more functions. */
    switch(currentInst.kind) {
        case mips_add: {
            /* Add two new add instructions using the original input operands */
            //TODO change the instructions so they are addu instead of addi
            instructionStruct firstDup;
            instructionStruct secondDup;
            
            //TODO need a good way to build/duplicate an instruction here in user.
            /* Transfer relevant information to the duplicated instructions  */
            firstDup.kind = currentInst.kind;
            firstDup.mnemonic = currentInst.mnemonic;
            firstDup.format = currentInst.format;
            firstDup.sourceRegisters = currentInst.sourceRegisters;
            //Destination will be different.
            firstDup.instructionConstant = currentInst.instructionConstant;
            firstDup.significantBits = currentInst.significantBits;
            firstDup.memoryReferenceSize = currentInst.memoryReferenceSize;
            firstDup.isSignedMemory = currentInst.isSignedMemory;
            //the address is not copied.
            /* Transfer relevant information to the duplicated instructions  */
            secondDup.kind = currentInst.kind;
            secondDup.mnemonic = currentInst.mnemonic;
            secondDup.format = currentInst.format;
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
            insertInstruction(secondMipsDup);
            
            /* another add to combine the result of two inserted adds,
                one more to combine the result of the original instruction
                result and the combined result. */
            instructionStruct accumulationOne;
            accumulationOne.kind = mips_addu;
            accumulationOne.mnemonic = "addu";
            accumulationOne.format = getInstructionFormat(mips_addu);
            /* Set the destination registers from duplicated instructions as source */
            accumulationOne.sourceRegisters.push_back(regOne);
            accumulationOne.sourceRegisters.push_back(regTwo);
            /* Reuse one of the symbolic registers as destination */
            accumulationOne.destinationRegisters.push_back(regOne);
            SgAsmMipsInstruction* accInst = buildInstruction(&accumulationOne);
            insertInstruction(accInst);
            
            /* use one of the generated register and set it to three
                so it can be used in the division. */
            instructionStruct denomInst;
            denomInst.kind = mips_addiu;
            denomInst.mnemonic = "addiu";
            denomInst.format = getInstructionFormat(mips_addiu);
            denomInst.instructionConstant = 0x3;
            SgAsmMipsInstruction* divConst = buildInstruction(&denomInst);
            insertInstruction(divConst);

            /* final accumulation instruction */
            instructionStruct accumulationFinal;
            accumulationFinal.kind = mips_addu;
            accumulationFinal.mnemonic = "addu";
            accumulationFinal.format = getInstructionFormat(mips_addu);
            /* Set the source registers, one symbolic and the destination register from
                the original instruction. */
            accumulationFinal.sourceRegisters.push_back(regOne);
            registerStruct orgInstDest = currentInst.destinationRegisters.back();
            accumulationFinal.sourceRegisters.push_back(orgInstDest);
            SgAsmMipsInstruction* accFinal = buildInstruction(&accumulationFinal);

            /* division instruction with the total sum of the adds
                that is divided by three. Giving us the average value.  */
            instructionStruct divInst;
            divInst.kind = mips_divu;
            divInst.mnemonic = "divu";
            divInst.format = getInstructionFormat(mips_divu);
            /* set the operands of the division */
            //divInst.sourceRegisters.push_back(

            /* Move the result from the special register */

        }
        case mips_addi: {

        }
    }
}

