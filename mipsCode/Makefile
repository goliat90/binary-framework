#Makefile for building test binaires
#This make file expects you to have ELDK-4.1 in your path
#with the CROSS_COMPILE environment variable set to mips_4KCle-


#Simple add compile
simpleAdd.out: mipsSimpleAdd.c
	mips-linux-gcc -static -O0 mipsSimpleAdd.c -o simpleAdd.out



#The all rule which compiles all the binaries.
all: simpleAdd.out

#Remove all binaries
clean:
	rm -f simpleAdd.out

