/* Functions for debugging the framework  */

/* header file */
#include "binaryDebug.hpp"

#include <bitset>

/* Forward declaration */
/* initfunction for register enum to string map.  */
std::map<mipsRegisterName, std::string> initRegStringMap();
/* adds registers to the stringstream object */
void printRegisters(std::stringstream* regStream, std::vector<registerStruct>* registers); 
/* Print instruction constants */
void printConstant(std::stringstream* conStream, instructionStruct* instStruct);
/*  Adds registers to the assembly print stream. */
void assemblyRegister(std::stringstream*, std::vector<registerStruct>*);
/*  Function that adds constants to the assembly string. */
void assemblyConstant(std::stringstream*, instructionStruct*);
/*  Handles building assembly stores. */
void assemblyStores(std::stringstream*, instructionStruct*);
/*  Replaces addresses in branch instructions with labels. */
void assemblyLabel(std::stringstream*, instructionStruct*);
/*  Initfunction for the register enum to assembly string map. */
std::map<mipsRegisterName, std::string> initAssemblyMap();

/* Map that is mapping enum to corresponding string */
static std::map<mipsRegisterName, std::string> assemblyMap = initAssemblyMap();
/*  Map tracking all the blocks address which are substituted
    with a label in the assembly printer. */
static std::map<rose_addr_t, std::string> addressToLabelMap;

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
        // print the instructions raw bytes
        SgUnsignedCharList rawBytes = mipsInst->get_raw_bytes();
        for(SgUnsignedCharList::reverse_iterator rawIter = rawBytes.rbegin();
            rawIter != rawBytes.rend(); ++rawIter) {
            std::bitset<8> charBits(*rawIter);
            std::cout << charBits << " ";
        }
        std::cout << std::endl;
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
               has instructionConstant, significantBits, memoryreferencesize */
            *conStream << "Constant: 0x" << std::hex << instStruct->instructionConstant << " ";
            *conStream << "SignificantBits: " << std::dec << instStruct->significantBits << " ";
            *conStream << "DataSize: " << std::dec << instStruct->memoryReferenceSize << " bits ";
            *conStream << "isSigned: " << instStruct->isSignedMemory;
            break;
        }
        case R_RD_RS_C:
        case I_RD_RS_C:
        case I_RS_RT_C:
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


/* print the instructions contained in a basicblock in assembly format. */
void printBasicBlockAsAssembly(SgAsmBlock* block) {
    //get the list of instructions in the block.
    SgAsmStatementPtrList* stmtlistPtr = &block->get_statementList();
    /*  block address. */
    rose_addr_t blockId = block->get_id();
    /* print the block number */
    std::cout << "#********** "
              << "Block: " << std::hex << blockId
              << " **********" << std::endl
              << std::dec; //dont print hex numbers in after this

    //TODO Insert label for the block. Make one or use existing.
    if (1 == addressToLabelMap.count(blockId)) {
        /*  there Is a label for the block. use it. */
        std::string label = addressToLabelMap.find(blockId)->second;
        std::cout << label << ":" << std::endl;
    } else {
        /*  There is no label for the block yet, create one. */
        int mapSize = addressToLabelMap.size();
        /*  build label string and print it. */
        std::stringstream labelStream;
        labelStream << "blockLabel_" << mapSize;
        std::cout << labelStream.str() << ":" << std::endl;
        /*  Save the label in the map. */
        addressToLabelMap.insert(std::pair<rose_addr_t, std::string>(blockId, labelStream.str()));
    }

    /* iterate through the statement list and print. */
    for(SgAsmStatementPtrList::iterator instIter = stmtlistPtr->begin();
        instIter != stmtlistPtr->end(); instIter++) {
        /* Cast the SgAsmStatement to SgAsmMipsInstruction */
        SgAsmMipsInstruction* mipsInst = isSgAsmMipsInstruction(*instIter);
        /*  Call the print instruction. */
        printAssemblyInstruction(mipsInst);
    }
    /* print some delimiter as well */
    std::cout << "#********** "
              << "End of Block: " << std::hex << block->get_id()
              << " **********" << std::endl
              << std::dec; //dont print hex numbers in hex after this
}

/*  Prints the instruction in assembly format. This is so it could be pasted in to a file. */
void printAssemblyInstruction(SgAsmMipsInstruction* mipsInst) {
    /* string stream object */
    std::stringstream assemblyStream;
    /*  Decode the instruction. */
    instructionStruct asmStruct = decodeInstruction(mipsInst);
    /*  Add the name to the of the instruction to the stream. */
    assemblyStream << asmStruct.mnemonic << "\t";
    /*  Check format of the instruction and depending on the format build
        the correct assembly format. */
    switch(asmStruct.format) {
        case R_RD_RS_RT:
            assemblyRegister(&assemblyStream, &asmStruct.destinationRegisters);
            assemblyStream << ", ";
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            break;
        case R_RD_RS_C:
            assemblyRegister(&assemblyStream, &asmStruct.destinationRegisters);
            assemblyStream << ", ";
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            assemblyStream << ", ";
            assemblyConstant(&assemblyStream, &asmStruct);
            break;
        case R_RD:
            assemblyRegister(&assemblyStream, &asmStruct.destinationRegisters);
            break;
        case R_RS_RT:
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            break;
        case R_RS:
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
        case R_NOP: {
            break;
        }
        case I_RD_RS_C:
            assemblyRegister(&assemblyStream, &asmStruct.destinationRegisters);
            assemblyStream << ", ";
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            assemblyStream << ", ";
            assemblyConstant(&assemblyStream, &asmStruct);
            break;
        case I_RD_MEM_RS_C:
            assemblyRegister(&assemblyStream, &asmStruct.destinationRegisters);
            assemblyStream << ", ";
            assemblyConstant(&assemblyStream, &asmStruct);

            assemblyStream << "(";
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            assemblyStream << ")";
            break;
        case I_RD_C:
            assemblyRegister(&assemblyStream, &asmStruct.destinationRegisters);
            assemblyStream << ", ";
            assemblyConstant(&assemblyStream, &asmStruct);
            break;
        case I_RS_RT_C:
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            assemblyStream << ", ";
            assemblyLabel(&assemblyStream, &asmStruct);
            break;
        case I_RS_MEM_RT_C:
            assemblyStores(&assemblyStream, &asmStruct);
            break;
        case I_RS_C:
            //TODO this is bgez, bgezal, bgtz, blez, bltz change constant for label.
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            assemblyStream << ", ";
            assemblyLabel(&assemblyStream, &asmStruct);
            break;
        case J_C:
            assemblyConstant(&assemblyStream, &asmStruct);
            break;
        case J_RS:
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            break;
        case J_RD_RS:
            assemblyRegister(&assemblyStream, &asmStruct.destinationRegisters);
            assemblyStream << ", ";
            assemblyRegister(&assemblyStream, &asmStruct.sourceRegisters);
            break;
        default: {
            //TODO throw error here?
            break;
        }
    }
    /*  Print out the block from the stream. */
    std::cout << assemblyStream.str() << std::endl;
}

/*  Add an register to the stream. */
void assemblyRegister(std::stringstream* regStream, std::vector<registerStruct>* registers) {
    /* add the registers to the register stream, iterate over the vector and add them. */
    for(std::vector<registerStruct>::iterator iter = registers->begin();
        iter != registers->end(); iter++) {
        /*  add register to the stream. */
        *regStream << assemblyMap.find((*iter).regName)->second;
        /*  Add comma if the there is another register. */
        if (registers->end() != iter+1 ) {
            *regStream << ", ";
        }
    }
}

/* Print instruction constants */
void assemblyConstant(std::stringstream* conStream, instructionStruct* instStruct) {
    /*  Truncate the offset so we dont have to manually remove higher chars. */
//    unsigned short int offset = static_cast<unsigned short int>(instStruct->instructionConstant);
    short int offset = static_cast<short int>(instStruct->instructionConstant);
    //std::stringstream offsetStr;
    //offsetStr << offset;
    //std::string conStr = boost::lexical_cast<std::string>(offset);
//    std::cout << conStr;
//    std::cout << offset;
    /*  Add constant to the stream. */
    *conStream << offset;
    //*conStream << conStr;
}

/*  Instead of a instruction address in the branch and jump instructions
    a label is set instead. */
void assemblyLabel(std::stringstream* labelStream, instructionStruct* instStruct) {
    /*  Get the address that is the target address. */
    rose_addr_t address = instStruct->instructionConstant;
    /*  Check if the address has a label already or needs one. */
    if (1 == addressToLabelMap.count(address)) {
        /*  There is a label, use it. */
        std::string label = addressToLabelMap.find(address)->second;
        *labelStream << label;
    } else {
        /*  No label available, generate one. */
        int mapSize = addressToLabelMap.size();
        /*  build label string and print it. */
        std::stringstream newLabelStream;
        newLabelStream << "blockLabel_" << mapSize;
        *labelStream << newLabelStream.str();
        /*  Save the label in the map. */
        addressToLabelMap.insert(std::pair<rose_addr_t, std::string>(address, newLabelStream.str()));
    }
}

/*  Handles creating all assembly store instructions. */
void assemblyStores(std::stringstream* assemblyStream, instructionStruct* asmStruct) {
    mipsRegisterName sourceReg = asmStruct->sourceRegisters.front().regName;
    mipsRegisterName baReg = asmStruct->sourceRegisters.back().regName;
    /*  Add the source register being stored. */
    *assemblyStream << assemblyMap.find(sourceReg)->second;
    *assemblyStream << ", ";
    /*  Add the offset constant*/
    assemblyConstant(assemblyStream, asmStruct);
    /*  Enclose the register holding the base address. */
    *assemblyStream << "(";
    *assemblyStream << assemblyMap.find(baReg)->second;
    *assemblyStream << ")";
}

/*  Init function for the register enum to assembly map. */
std::map<mipsRegisterName, std::string> initAssemblyMap() {
    std::map<mipsRegisterName, std::string> assemblyMap;
    /* inserting all register names with their matching enum */
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(zero,"$zero"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(at, "$1"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(v0, "$2"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(v1, "$3"));

    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(a0, "$4"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(a1, "$5"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(a2, "$6"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(a3, "$7"));

    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t0, "$8"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t1, "$9"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t2, "$10"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t3, "$11"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t4, "$12"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t5, "$13"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t6, "$14"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t7, "$15"));

    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s0, "$16"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s1, "$17"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s2, "$18"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s3, "$19"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s4, "$20"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s5, "$21"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s6, "$22"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(s7, "$23"));

    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t8, "$24"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(t9, "$25"));

    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(k0, "$26"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(k1, "$27"));

    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(gp, "$gp"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(sp, "$sp"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(fp, "$fp"));
    assemblyMap.insert(std::pair<mipsRegisterName, std::string>(ra, "$ra"));
    /* return the filled map */
    return assemblyMap;
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

/*  Function to clear the address to label map. */
void clearAddressLabelMap() {
    addressToLabelMap.clear();
}

