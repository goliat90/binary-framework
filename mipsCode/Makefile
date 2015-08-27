#Makefile for building test binaires
#This make file expects you to have ELDK-4.1 in your path
#with the CROSS_COMPILE environment variable set to mips_4KCle-

#The all rule which compiles all the binaries.
all: mipsSimpleAdd.out mipsAdvancedAdd.out mipsDivisionTest.out \
		mipsModulusTest.out mipsDivisionMod.out mipsLogical.out \
		mipsShifter.out mipsNumCounter.out mipsOneBB.out mipsIfElse.out

#Simple add compile
mipsSimpleAdd.out: mipsSimpleAdd.c
	mips-linux-gcc -static -march=r3000 -O1 mipsSimpleAdd.c -o mipsSimpleAdd.out

#Advanced add.
mipsAdvancedAdd.out: mipsAdvancedAdd.c
	mips-linux-gcc -static -march=r3000 -O1 mipsAdvancedAdd.c -o mipsAdvancedAdd.out

#Mips division test.
mipsDivisionTest.out: mipsDivisionTest.c
	mips-linux-gcc -static -march=r3000 -O1 mipsDivisionTest.c -o mipsDivisionTest.out

#Mips modulus test.
mipsModulusTest.out: mipsModulusTest.c
	mips-linux-gcc -static -march=r3000 -O1 mipsModulusTest.c -o mipsModulusTest.out

#Mips modulus and mod test.
mipsDivisionMod.out: mipsDivisionMod.c
	mips-linux-gcc -static -march=r3000 -O1 mipsDivisionMod.c -o mipsDivisionMod.out

#Mips logical test.
mipsLogical.out: mipsLogical.c
	mips-linux-gcc -static -march=r3000 -O1 mipsLogical.c -o mipsLogical.out

#Mips shift test.
mipsShifter.out: mipsShifter.c
	mips-linux-gcc -static -march=r3000 -O1 mipsShifter.c -o mipsShifter.out

#One basic block code.
mipsOneBB.out: mipsOneBB.c
	mips-linux-gcc -static -march=r3000 -O1 mipsOneBB.c -o mipsOneBB.out

#If else code that test cfg layout.
mipsIfElse.out: mipsIfElse.c
	mips-linux-gcc -static -march=r3000 -O1 mipsIfElse.c -o mipsIfElse.out

#ascii counter
mipsNumCounter.out: mipsNumCounter.c
	mips-linux-gcc -static -march=r3000 -O2 mipsNumCounter.c -o mipsNumCounter.out


#Remove all binaries
clean:
	rm -f *.out

