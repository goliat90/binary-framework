/* Functions for debugging the framework  */

/* header file */
#include "binaryDebug.hpp"

/* Forward declaration */
/* initfunction for register enum to string map.  */
std::map<mipsRegisterName, std::string> initRegStringMap();
/* adds registers to the stringstream object */
void printRegisters(std::stringstream* regStream, std::vector<registerStruct>* registers); 
/* Print instruction constants */
void printConstant(std::stringstream* conStream, instructionStruct* instStruct);

/* print the instructions contained in a basicblock */
void printBasicBlockInstructions(SgAsmBlock* block) {
    //get the list of instructions in the block.
    SgAsmStatementPtrList* stmtlistPtr = &block->get_statementList();
    /* print the block number */
    std::cout << "********** "
              << "Block: " << std::hex << block->get_id()
              << " **********" << std::endl
              << std::dec; //dont print hex numbers in after this

    /* iterate through the statement list and print. */
    for(SgAsmStatementPtrList::iterator instIter = stmtlistPtr->begin();
        instIter != stmtlistPtr->end(); instIter++) {
        /* Cast the SgAsmStatement to SgAsmMipsInstruction */
        SgAsmMipsInstruction* mipsInst = isSgAsmMipsInstruction(*instIter);
        /* decode the instruction and print the information */
        instructionStruct instruction = decodeInstruction(mipsInst);
        /* call print instruction */
        printInstruction(&instruction);
    }
    /* print some delimiter as well */
    std::cout << "********** "
              << "End of Block: " << std::hex << block->get_id()
              << " **********" << std::endl
              << std::dec; //dont print hex numbers in hex after this
}


/*  Wrapper of print instruction, decodes the instruction then
    calls the function below. */
void printInstruction(SgAsmMipsInstruction* mipsInst) {
    /*  Decode instruction. */
    instructionStruct mipsStruct = decodeInstruction(mipsInst);
    /*  Call the other. */
    printInstruction(&mipsStruct);
}

/* Prints out relevant information about a instruction */
void printInstruction(instructionStruct* instStruct) {
    /* string stream object */
    std::stringstream instStream;
    /* insert address, nemonic */
    instStream << std::hex << instStruct->address << ": " << instStruct->mnemonic << " ";
    /* add registers, first output then input registers. */ 
    printRegisters(&instStream, &instStruct->destinationRegisters);
    printRegisters(&instStream, &instStruct->sourceRegisters);
    /* check if there are any constant, if so then include them. */
    printConstant(&instStream, instStruct);
    /* print the instructions information */
    std::cout << instStream.str() << std::endl;
    
}

/* Print instruction constants */
void printConstant(std::stringstream* conStream, instructionStruct* instStruct) {
    /* include constants if it is present in the instruction. */
    switch(instStruct->format) {
        case I_RD_MEM_RS_C:
        case I_RS_MEM_RT_C: {
            /* instructions that work with memory and has constant 
               has instructionConstatn, significantBits, memoryreferencesize */
            *conStream << "Constant: 0x" << std::hex << instStruct->instructionConstant << " ";
            *conStream << "SignificantBits: " << std::dec << instStruct->significantBits << " ";
            *conStream << "DataSize: " << std::dec << instStruct->memoryReferenceSize << " bits ";
            *conStream << "isSigned: " << instStruct->isSignedMemory;
            break;
        }
        case R_RD_RS_C:
        case I_RD_RS_C:
        case I_RD_C:
        case I_RS_C: {
            /* instructions with regular constant
               has instructionConstant, significantBits */
            *conStream << "Constant: 0x" << std::hex << instStruct->instructionConstant << " ";
            *conStream << "SignificantBits: " << std::dec << instStruct->significantBits << " ";
        break;
        }
        default: {
            //std::cout << "skipped constants on address: " << std::hex << instStruct->address << std::endl;
        }
    }
    
}


/*  Function to get a register name as a string. */
std::string getRegisterString(mipsRegisterName reg) {
    /* Map that is mapping enum to corresponding string */
    static std::map<mipsRegisterName, std::string> registerMap = initRegStringMap();
    /*  Find the string for the register. */
    std::map<mipsRegisterName, std::string>::iterator regIter = registerMap.find(reg);
    /*  Check that the the iterator is not end */
    if (regIter != registerMap.end()) {
        return regIter->second;
    } else {
        return "Invalid Reg Name";
    }
}

/* adds registers to the stringstream object */
void printRegisters(std::stringstream* regStream, std::vector<registerStruct>* registers) {
    /* Map that is mapping enum to corresponding string */
    static std::map<mipsRegisterName, std::string> registerMap = initRegStringMap();
    /* add the registers to the register stream, iterate over the vector and add them. */
    for(std::vector<registerStruct>::iterator iter = registers->begin();
        iter != registers->end(); iter++) {
        /* take out the register struct, check if it is symbolic or physical */
        registerStruct regS = *iter;
        if (regS.regName != symbolic_reg) {
            /* the register is real, get the string for it.*/
            *regStream << registerMap.find(regS.regName)->second;
        } else if (regS.regName == symbolic_reg) {
            /* the register is symbolic */
            *regStream << "sym_" << std::dec << regS.symbolicNumber;
        } else {
            /* the register is neither physical or symbolic. error */
            *regStream << "Non symbolic or physical register.";
        }
        /* Add a space between register names */
        *regStream << " ";
    }
}


/* initfunction for register enum to string map.  */
std::map<mipsRegisterName, std::string> initRegStringMap() {
    std::map<mipsRegisterName, std::string> regMap;
    /* inserting all register names with their matching enum */
    regMap.insert(std::pair<mipsRegisterName, std::string>(zero,"zero"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(at, "at"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(v0, "v0"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(v1, "v1"));

    regMap.insert(std::pair<mipsRegisterName, std::string>(a0, "a0"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(a1, "a1"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(a2, "a2"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(a3, "a3"));

    regMap.insert(std::pair<mipsRegisterName, std::string>(t0, "t0"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t1, "t1"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t2, "t2"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t3, "t3"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t4, "t4"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t5, "t5"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t6, "t6"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t7, "t7"));

    regMap.insert(std::pair<mipsRegisterName, std::string>(s0, "s0"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(s1, "s1"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(s2, "s2"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(s3, "s3"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(s4, "s4"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(s5, "s5"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(s6, "s6"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(s7, "s7"));

    regMap.insert(std::pair<mipsRegisterName, std::string>(t8, "t8"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(t9, "t9"));
                                                                  
    regMap.insert(std::pair<mipsRegisterName, std::string>(k0, "k0"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(k1, "k1"));
                                                                  
    regMap.insert(std::pair<mipsRegisterName, std::string>(gp, "gp"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(sp, "sp"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(fp, "fp"));
    regMap.insert(std::pair<mipsRegisterName, std::string>(ra, "ra"));
    /* return the filled map */
    return regMap;
}
