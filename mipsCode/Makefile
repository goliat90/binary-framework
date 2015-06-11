#Makefile for building test binaires
#This make file expects you to have ELDK-4.1 in your path
#with the CROSS_COMPILE environment variable set to mips_4KCle-

#The all rule which compiles all the binaries.
all: numCounter.out simpleAdd.out

#Simple add compile
simpleAdd.out: mipsSimpleAdd.c
	mips-linux-gcc -static -O2 mipsSimpleAdd.c -o simpleAdd.out

#ascii counter
numCounter.out: mipsNumCounter.c
	mips-linux-gcc -static -O2 mipsNumCounter.c -o numCounter.out


#Remove all binaries
clean:
	rm -f simpleAdd.out
	rm -f numCounter.out

