ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
endif


# this could be conditional as well. 

# Default make rule to use
all: CFGdot


# Compile the CFGdot using the file CFGdot.C
CFGdot.lo: CFGdot.C
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF)  -c -o CFGdot.lo ./src/CFGdot.C


CFGdot: CFGdot.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o CFGdot.out CFGdot.lo 

clean:
	rm -rf .libs
	rm -f CFGdot
	rm -f CFGdot.lo
	rm -f CFGdot.o


