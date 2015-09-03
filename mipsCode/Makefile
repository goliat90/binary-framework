#Makefile for building test binaires
#This make file expects you to have ELDK-4.1 in your path
#with the CROSS_COMPILE environment variable set to mips_4KCle-

# Path variables
SRC = ./src
INCLUDE = ./include
OBJ = ./objectfiles
ASM = ./assembly

# Vpath, specifies which folders to look for files regarding dependency.
VPATH = src:includes

#The all rule which compiles all the binaries.
all:	simpleAdd.out \
		oneBasic.out
#		mipsAdvancedAdd.out mipsDivisionTest.out \
#		mipsModulusTest.out mipsDivisionMod.out mipsLogical.out \
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
	-o simpleAdd.out


# Compile function object.
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
	-o oneBasic.out

#Link to make output file but with assembly of the function.
oneBasicAsm.out: oneBasicMain
	mips-linux-gcc -static $(ASM)/oneBasic.s $(OBJ)/oneBasicMain.o \
	-o oneBasicAsm.out

# Modulus test binary
# 
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
	-o modulusTest.out

modulusTestAsm.out: modulusTestMain
	mips-linux-gcc -static $(ASM)/modulusTest.s $(OBJ)/modulusTestMain.o \
	-o modulusTestAsm.out



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


#Remove all binaries
clean:
	rm -f *.out
	rm -f $(OBJ)/*

