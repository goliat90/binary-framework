ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
endif

SRCDIR = ./src

# testing adding more folders to vpath
VPATH = src:include

all: linking 

mipsISA.lo: mipsISA.cpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o mipsISA.lo \
	$(SRCDIR)/mipsISA.cpp

symbolicRegisters.lo: symbolicRegisters.cpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o symbolicRegisters.lo \
	$(SRCDIR)/symbolicRegisters.cpp

framework.lo: binaryRewriter.cpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o binaryRewriter.lo \
	$(SRCDIR)/binaryRewriter.cpp

test.lo: test.cpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o test.lo \
	$(SRCDIR)/test.cpp

linking: framework.lo test.lo symbolicRegisters.lo mipsISA.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o test.out test.lo binaryRewriter.lo symbolicRegisters.lo

	

clean:
	rm -rf .libs
	rm -f test.lo
	rm -f binaryRewriter.lo
	rm -f symbolicRegisters.lo
	rm -f mipsISA.lo
	rm -f test.out
	rm -f test.o
	rm -f binaryRewriter.o
	rm -f symbolicRegisters.o
	rm -f mipsISA.o


