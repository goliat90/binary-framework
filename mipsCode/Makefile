#Makefile for building test binaires
#This make file expects you to have ELDK-4.1 in your path
#with the CROSS_COMPILE environment variable set to mips_4KCle-

# Path variables
SRC = ./src
INCLUDE = ./include
OBJ = ./objectfiles
ASM = ./assembly
BIN = ./binaries

CXXFLAGS = -static -march=r3000 -O1 -mno-check-zero-division -fno-delayed-branch

# Vpath, specifies which folders to look for files regarding dependency.
VPATH = src:includes:binaries

#The all rule which compiles all the binaries.
all:	simpleAdd.out \
		oneBasic.out \
		modulusTest.out \
		logicalTest.out \
		advancedAdd.out \
		shiftTest.out \
		ifElseTest.out \
		divisionModulusTest.out \
        divisionTest.out \
        stackAllocationTest.out
#		mipsAdvancedAdd.out mipsDivisionTest.out \
#		mipsDivisionMod.out mipsLogical.out \
#		mipsShifter.out mipsNumCounter.out mipsOneBB.out mipsIfElse.out

# Compile the simple add to object.
simpleAdd: simpleAdd.c
	mips-linux-gcc -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/simpleAdd.c \
    -o $(OBJ)/simpleAdd.o

# Compile the simple main object
simpleAddMain: simpleAddMain.c
	mips-linux-gcc -static -march=r3000 -O1 -I$(INCLUDE) -c $(SRC)/simpleAddMain.c \
	-o $(OBJ)/simpleAddMain.o

#Link together to an out file. 
simpleAdd.out: simpleAdd simpleAddMain
	mips-linux-gcc -static $(OBJ)/simpleAdd.o $(OBJ)/simpleAddMain.o \
	-o $(BIN)/simpleAdd.out


# One basic block test binary.
oneBasic: oneBasic.c
	mips-linux-gcc -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/oneBasic.c \
	-o $(OBJ)/oneBasic.o

# Assembly file for oneBasic
oneBasicAssembly: oneBasic.c
	mips-linux-gcc -S -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/oneBasic.c \
	-o $(ASM)/oneBasic.s

# Compile the simple main object
oneBasicMain: oneBasicMain.c
	mips-linux-gcc -static -march=r3000 -O1 -I$(INCLUDE) -c $(SRC)/oneBasicMain.c \
	-o $(OBJ)/oneBasicMain.o

#Link together to an out file. 
oneBasic.out: oneBasic oneBasicMain
	mips-linux-gcc -static $(OBJ)/oneBasic.o $(OBJ)/oneBasicMain.o \
	-o $(BIN)/oneBasic.out

#Link to make output file but with assembly of the function.
oneBasicAsm.out: oneBasicMain
	mips-linux-gcc -static $(ASM)/oneBasic.s $(OBJ)/oneBasicMain.o \
	-o oneBasicAsm.out


# Modulus test binary
modulusTest: modulusTest.c
	mips-linux-gcc -static -march=r3000 -O1 -mno-check-zero-division -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/modulusTest.c \
	-o $(OBJ)/modulusTest.o

modulusTestAssembly: modulusTest.c
	mips-linux-gcc -S -static -march=r3000 -O1 -mno-check-zero-division -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/modulusTest.c \
	-o $(ASM)/modulusTest.s

modulusTestMain: modulusTestMain.c
	mips-linux-gcc -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/modulusTestMain.c \
	-o $(OBJ)/modulusTestMain.o

modulusTest.out: modulusTest modulusTestMain
	mips-linux-gcc -static $(OBJ)/modulusTest.o $(OBJ)/modulusTestMain.o \
	-o $(BIN)/modulusTest.out

modulusTestAsm.out: modulusTestMain
	mips-linux-gcc -static $(ASM)/modulusTest.s $(OBJ)/modulusTestMain.o \
	-o modulusTestAsm.out


# Logical test binary.
logicalTest: logicalTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/logicalTest.c \
	-o $(OBJ)/logicalTest.o

logicalTestMain: logicalTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/logicalTestMain.c \
	-o $(OBJ)/logicalTestMain.o

logicalTest.out: logicalTest logicalTestMain
	mips-linux-gcc -static $(OBJ)/logicalTest.o $(OBJ)/logicalTestMain.o \
	-o $(BIN)/logicalTest.out
	

# Advanced add test binary. 
advancedAdd: advancedAdd.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/advancedAdd.c \
	-o $(OBJ)/advancedAdd.o

advancedAddMain: advancedAddMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/advancedAddMain.c \
	-o $(OBJ)/advancedAddMain.o

advancedAdd.out: advancedAdd advancedAddMain
	mips-linux-gcc -static $(OBJ)/advancedAdd.o $(OBJ)/advancedAddMain.o \
	-o $(BIN)/advancedAdd.out


# Shift test binary.
shiftTest: shiftTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/shiftTest.c \
	-o $(OBJ)/shiftTest.o

shiftTestMain: shiftTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/shiftTestMain.c \
	-o $(OBJ)/shiftTestMain.o

shiftTest.out: shiftTest shiftTestMain
	mips-linux-gcc -static $(OBJ)/shiftTest.o $(OBJ)/shiftTestMain.o \
	-o $(BIN)/shiftTest.out


# if else test binary.
ifElseTest: ifElseTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/ifElseTest.c \
	-o $(OBJ)/ifElseTest.o

ifElseTestMain: ifElseTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/ifElseTestMain.c \
	-o $(OBJ)/ifElseTestMain.o

ifElseTest.out: ifElseTest ifElseTestMain
	mips-linux-gcc -static $(OBJ)/ifElseTest.o $(OBJ)/ifElseTestMain.o \
	-o $(BIN)/ifElseTest.out


# Combined division modulus test binary.
divisionModulusTest: divisionModulusTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionModulusTest.c \
	-o $(OBJ)/divisionModulusTest.o

divisionModulusTestMain: divisionModulusTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionModulusTestMain.c \
	-o $(OBJ)/divisionModulusTestMain.o

divisionModulusTest.out: divisionModulusTest divisionModulusTestMain
	mips-linux-gcc -static $(OBJ)/divisionModulusTest.o $(OBJ)/divisionModulusTestMain.o \
	-o $(BIN)/divisionModulusTest.out


# Divison test binary.
divisionTest: divisionTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionTest.c \
	-o $(OBJ)/divisionTest.o

divisionTestMain: divisionTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionTestMain.c \
	-o $(OBJ)/divisionTestMain.o

divisionTest.out: divisionTest divisionTestMain
	mips-linux-gcc -static $(OBJ)/divisionTest.o $(OBJ)/divisionTestMain.o \
	-o $(BIN)/divisionTest.out


# Stack allocation test binary.
stackAllocationTest: stackAllocationTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/stackAllocationTest.c \
	-o $(OBJ)/stackAllocationTest.o

stackAllocationTestMain: stackAllocationTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/stackAllocationTestMain.c \
	-o $(OBJ)/stackAllocationTestMain.o

stackAllocationTest.out: stackAllocationTest stackAllocationTestMain
	mips-linux-gcc -static $(OBJ)/stackAllocationTest.o	$(OBJ)/stackAllocationTestMain.o \
	-o $(BIN)/stackAllocationTest.out


##Advanced add.
#mipsAdvancedAdd.out: mipsAdvancedAdd.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsAdvancedAdd.c -o mipsAdvancedAdd.out
#
##Mips division test.
#mipsDivisionTest.out: mipsDivisionTest.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsDivisionTest.c -o mipsDivisionTest.out
#
##Mips modulus test.
#mipsModulusTest.out: mipsModulusTest.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsModulusTest.c -o mipsModulusTest.out
#
##Mips modulus and mod test.
#mipsDivisionMod.out: mipsDivisionMod.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsDivisionMod.c -o mipsDivisionMod.out
#
##Mips logical test.
#mipsLogical.out: mipsLogical.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsLogical.c -o mipsLogical.out
#
##Mips shift test.
#mipsShifter.out: mipsShifter.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsShifter.c -o mipsShifter.out
#
##One basic block code.
#mipsOneBB.out: mipsOneBB.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsOneBB.c -o mipsOneBB.out
#
##If else code that test cfg layout.
#mipsIfElse.out: mipsIfElse.c
#	mips-linux-gcc -static -march=r3000 -O1 mipsIfElse.c -o mipsIfElse.out
#
##ascii counter
#mipsNumCounter.out: mipsNumCounter.c
#	mips-linux-gcc -static -march=r3000 -O2 mipsNumCounter.c -o mipsNumCounter.out


#Remove all binaries and object files.
clean:
	rm -f $(BIN)/*
	rm -f $(OBJ)/*

