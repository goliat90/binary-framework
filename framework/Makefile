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


cfgHandler.lo: cfgHandler.cpp cfgHandler.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o cfgHandler.lo \
	$(SRCDIR)/cfgHandler.cpp

binaryDebug.lo: binaryDebug.cpp binaryDebug.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o binaryDebug.lo \
	$(SRCDIR)/binaryDebug.cpp

mipsISA.lo: mipsISA.cpp mipsISA.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o mipsISA.lo \
	$(SRCDIR)/mipsISA.cpp

symbolicRegisters.lo: symbolicRegisters.cpp symbolicRegisters.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o symbolicRegisters.lo \
	$(SRCDIR)/symbolicRegisters.cpp

framework.lo: binaryRewriter.cpp binaryRewriter.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o binaryRewriter.lo \
	$(SRCDIR)/binaryRewriter.cpp

test.lo: test.cpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o test.lo \
	$(SRCDIR)/test.cpp

linking: framework.lo test.lo symbolicRegisters.lo mipsISA.lo binaryDebug.lo cfgHandler.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o test.out test.lo binaryRewriter.lo symbolicRegisters.lo \
	binaryDebug.lo mipsISA.lo cfgHandler.lo

	

clean:
	rm -rf .libs
	rm -f test.out
	rm -f binaryDebug.lo
	rm -f test.lo
	rm -f binaryRewriter.lo
	rm -f symbolicRegisters.lo
	rm -f mipsISA.lo
	rm -f cfgHandler.lo
	rm -f binaryDebug.o
	rm -f test.o
	rm -f binaryRewriter.o
	rm -f symbolicRegisters.o
	rm -f mipsISA.o
	rm -f cfgHandler.o


