/* Header file for binaryRewriter.cpp */

#ifndef BINARY_REWRITER_H
#define BINARY_REWRITER_H

//Framework headers
#include "InstructionInformation.hpp"

// Boost lib headers
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vector_as_graph.hpp>

// Rose headers
#include "rose.h"

/* Class declaration */
class BinaryRewriter {
    public:
        //Default constructor.
        BinaryRewriter() {};
        //Constructor with file
        BinaryRewriter(int, char**);
        //Takes argc and argv then creates an ast and cfg.
        void initialize(int ,char**);
        //Configure register allocation
        void selectRegisterAllocation();
        //Configure instruction scheduling
        void selectInstructionScheduling();
        //Function to begin rewriting
        void traverseBinary();

        //Add instruction
        void insertInstruction(SgAsmStatement*); //need an argument here.
        //Returns the input registers of the current instruction.
        std::string inputRegisters();
        //Returns the output registers of the current register
        std::string outputRegisters();

        //Remove current instruction
        void removeInstruction(); 
        //Next instruction, copies the currently inspected instruction.
        void saveInstruction();
        //Get the instruction enum of the currently inspected instruction.
        MipsInstructionKind getInstructionKind();
        //Return the operand list of the current instruction, result and source regs.
        SgAsmExpressionPtrList getInstructionOperands();
        //return the mnemonic string
        std::string getInstructionMnemonic();

        //Function that provides symbolic register labels.
        void generateSymbolicRegister(); //need to check how operands/registers are typed.
        //Virtual function that the user can change in his framework extension.
        virtual void transformDecision(SgAsmStatement*);
        //Print statistics.
        void printInformation();

    private:
        //typedefs
        typedef rose::BinaryAnalysis::ControlFlow::Graph Cfg;
        typedef boost::graph_traits<Cfg>::vertex_iterator CfgVIter;
        //map type for the property map in the cfg that contains the basic blocks.
        typedef boost::property_map<Cfg, boost::vertex_name_t>::type basicBlockMap;

        // -------- instruction struct --------
        // Contains information that is useful for the framework about
        // the current instruction.
        struct instructionInformation {
            //nmemonic enum.
            MipsInstructionKind kind;
            //nmemonic string.
            std::string mnemonic;
            //input register(s)
            std::vector<std::pair<std::string, const RegisterDescriptor*> > inregs;
            //output register(s)
            std::vector<std::pair<std::string, const RegisterDescriptor*> > outregs;
            //operands list
            SgAsmOperandList* operandsList; 
            //operands list, but as SgAsmExpressionPtrList, one level lower than previous.
            //SgAsmExpressionPtrList* operandExpressionList;
            //if the instruction uses a constant then save it.
            uint64_t instructionConstant;
            //The number of bits/size of the memory reference
            int memoryReferenceSize;
            //address of the instruction, if the instruction is inserted then it
            //is a temporary value or invalid.
            rose_addr_t address;
        };
        //Private variables.
        //Pointer to the project AST 
        SgProject* binaryProjectPtr;
        //Register dictionary.
        const RegisterDictionary* mipsRegisters; 
        //Control flow graph pointer.
        Cfg* CfgPtr;
        //Shadow statement list. This list will be swaped with the statementlist
        //att the end of traversing a basic blocks statement list. (vector)
        SgAsmStatementPtrList* shadowStatementListPtr;
        //Current instruction being inspected.
        SgAsmMipsInstruction* inspectedInstruction;
        //Inspected instruction struct with information
        instructionInformation instInfo;
        //number of decisions made
        int decisionsMade;

        // -------- Functions --------
        //block traversal
        void blockTraversal();
        //function traversal
        void functionTraversal();
        //deconstruct the current instruction to provide information.
        void deconstructInstruction();
        //decode the instruction operands
        void decodeOperands();
        //recursive decoding function
        void decodeExpression(int, SgAsmExpression*);

};

#endif 

