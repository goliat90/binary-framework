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
VPATH = src:includes:binaries:objectfiles

.PHONY: all clean

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
        stackAllocationTest.out \
		median.out

# Compile the simple add to object.
simpleAdd.o: simpleAdd.c
	mips-linux-gcc -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/simpleAdd.c \
    -o $(OBJ)/simpleAdd.o

# Compile the simple main object
simpleAddMain.o: simpleAddMain.c
	mips-linux-gcc -static -march=r3000 -O1 -I$(INCLUDE) -c $(SRC)/simpleAddMain.c \
	-o $(OBJ)/simpleAddMain.o

#Link together to an out file. 
simpleAdd.out: simpleAdd.o simpleAddMain.o
	mips-linux-gcc -static $(OBJ)/simpleAdd.o $(OBJ)/simpleAddMain.o \
	-o $(BIN)/simpleAdd.out


# One basic block test binary.
oneBasic.o: oneBasic.c
	mips-linux-gcc -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/oneBasic.c \
	-o $(OBJ)/oneBasic.o

# Assembly file for oneBasic
oneBasicAssembly: oneBasic.c
	mips-linux-gcc -S -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/oneBasic.c \
	-o $(ASM)/oneBasic.s

# Compile the simple main object
oneBasicMain.o: oneBasicMain.c
	mips-linux-gcc -static -march=r3000 -O1 -I$(INCLUDE) -c $(SRC)/oneBasicMain.c \
	-o $(OBJ)/oneBasicMain.o

#Link together to an out file. 
oneBasic.out: oneBasic.o oneBasicMain.o
	mips-linux-gcc -static $(OBJ)/oneBasic.o $(OBJ)/oneBasicMain.o \
	-o $(BIN)/oneBasic.out

#Link to make output file but with assembly of the function.
oneBasicAsm.out: oneBasicMain
	mips-linux-gcc -static $(ASM)/oneBasic.s $(OBJ)/oneBasicMain.o \
	-o oneBasicAsm.out


# Modulus test binary
modulusTest.o: modulusTest.c
	mips-linux-gcc -static -march=r3000 -O1 -mno-check-zero-division -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/modulusTest.c \
	-o $(OBJ)/modulusTest.o

modulusTestAssembly: modulusTest.c
	mips-linux-gcc -S -static -march=r3000 -O1 -mno-check-zero-division -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/modulusTest.c \
	-o $(ASM)/modulusTest.s

modulusTestMain.o: modulusTestMain.c
	mips-linux-gcc -static -march=r3000 -O1 -fno-delayed-branch -I$(INCLUDE) -c $(SRC)/modulusTestMain.c \
	-o $(OBJ)/modulusTestMain.o

modulusTest.out: modulusTest.o modulusTestMain.o
	mips-linux-gcc -static $(OBJ)/modulusTest.o $(OBJ)/modulusTestMain.o \
	-o $(BIN)/modulusTest.out

modulusTestAsm.out: modulusTestMain.o
	mips-linux-gcc -static $(ASM)/modulusTest.s $(OBJ)/modulusTestMain.o \
	-o modulusTestAsm.out


# Logical test binary.
logicalTest.o: logicalTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/logicalTest.c \
	-o $(OBJ)/logicalTest.o

logicalTestMain.o: logicalTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/logicalTestMain.c \
	-o $(OBJ)/logicalTestMain.o

logicalTest.out: logicalTest.o logicalTestMain.o
	mips-linux-gcc -static $(OBJ)/logicalTest.o $(OBJ)/logicalTestMain.o \
	-o $(BIN)/logicalTest.out
	

# Advanced add test binary. 
advancedAdd.o: advancedAdd.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/advancedAdd.c \
	-o $(OBJ)/advancedAdd.o

advancedAddMain.o: advancedAddMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/advancedAddMain.c \
	-o $(OBJ)/advancedAddMain.o

advancedAdd.out: advancedAdd.o advancedAddMain.o
	mips-linux-gcc -static $(OBJ)/advancedAdd.o $(OBJ)/advancedAddMain.o \
	-o $(BIN)/advancedAdd.out


# Shift test binary.
shiftTest.o: shiftTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/shiftTest.c \
	-o $(OBJ)/shiftTest.o

shiftTestMain.o: shiftTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/shiftTestMain.c \
	-o $(OBJ)/shiftTestMain.o

shiftTest.out: shiftTest.o shiftTestMain.o
	mips-linux-gcc -static $(OBJ)/shiftTest.o $(OBJ)/shiftTestMain.o \
	-o $(BIN)/shiftTest.out


# if else test binary.
ifElseTest.o: ifElseTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/ifElseTest.c \
	-o $(OBJ)/ifElseTest.o

ifElseTestMain.o: ifElseTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/ifElseTestMain.c \
	-o $(OBJ)/ifElseTestMain.o

ifElseTest.out: ifElseTest.o ifElseTestMain.o
	mips-linux-gcc -static $(OBJ)/ifElseTest.o $(OBJ)/ifElseTestMain.o \
	-o $(BIN)/ifElseTest.out


# Combined division modulus test binary.
divisionModulusTest.o: divisionModulusTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionModulusTest.c \
	-o $(OBJ)/divisionModulusTest.o

divisionModulusTestMain.o: divisionModulusTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionModulusTestMain.c \
	-o $(OBJ)/divisionModulusTestMain.o

divisionModulusTest.out: divisionModulusTest.o divisionModulusTestMain.o
	mips-linux-gcc -static $(OBJ)/divisionModulusTest.o $(OBJ)/divisionModulusTestMain.o \
	-o $(BIN)/divisionModulusTest.out


# Divison test binary.
divisionTest.o: divisionTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionTest.c \
	-o $(OBJ)/divisionTest.o

divisionTestMain.o: divisionTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/divisionTestMain.c \
	-o $(OBJ)/divisionTestMain.o

divisionTest.out: divisionTest.o divisionTestMain.o
	mips-linux-gcc -static $(OBJ)/divisionTest.o $(OBJ)/divisionTestMain.o \
	-o $(BIN)/divisionTest.out


# Stack allocation test binary.
stackAllocationTest.o: stackAllocationTest.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/stackAllocationTest.c \
	-o $(OBJ)/stackAllocationTest.o

stackAllocationTestMain.o: stackAllocationTestMain.c
	mips-linux-gcc $(CXXFLAGS) -I$(INCLUDE) -c $(SRC)/stackAllocationTestMain.c \
	-o $(OBJ)/stackAllocationTestMain.o

stackAllocationTest.out: stackAllocationTest.o stackAllocationTestMain.o
	mips-linux-gcc -static $(OBJ)/stackAllocationTest.o	$(OBJ)/stackAllocationTestMain.o \
	-o $(BIN)/stackAllocationTest.out


# Median test code.
median.out: median.c
	mips-linux-gcc $(CXXFLAGS) $(SRC)/median.c -o $(BIN)/median.out


#Remove all binaries and object files.
clean:
	rm -f $(BIN)/*
	rm -f $(OBJ)/*

