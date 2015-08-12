#Makefile for building test binaires
#This make file expects you to have ELDK-4.1 in your path
#with the CROSS_COMPILE environment variable set to mips_4KCle-

#The all rule which compiles all the binaries.
all: numCounter.out simpleAdd.out advancedAdd.out

#Simple add compile
simpleAdd.out: mipsSimpleAdd.c
	mips-linux-gcc -static -march=r3000 -O1 mipsSimpleAdd.c -o simpleAdd.out

#Advanced add.
advancedAdd.out: mipsAdvancedAdd.c
	mips-linux-gcc -static -march=r3000 -O1 mipsAdvancedAdd.c -o advancedAdd.out

#ascii counter
numCounter.out: mipsNumCounter.c
	mips-linux-gcc -static -march=r3000 -O2 mipsNumCounter.c -o numCounter.out


#Remove all binaries
clean:
	rm -f simpleAdd.out
	rm -f numCounter.out
	rm -f advancedAdd.out

