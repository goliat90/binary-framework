ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
endif

SRCDIR = ./src

# Default make rule to use
#all: CFGdot
#
#
## Compile the CFGdot using the file CFGdot.C
#CFGdot.lo:
#	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF)  -c -o CFGdot.lo CFGdot.C
#
#
#CFGdot: CFGdot.lo
#	libtool --mode=link g++ $(ROSE_LIBS) -o CFGdot.out CFGdot.lo 
#

all: linking 

framework.lo:
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o binaryRewriter.lo $(SRCDIR)/binaryRewriter.cpp

test.lo:
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o test.lo $(SRCDIR)/test.cpp

linking: framework.lo test.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o test.out test.lo binaryRewriter.lo

	

clean:
	rm -rf .libs
	rm -f test.lo
	rm -f binaryRewriter.lo
	rm -f test.out


