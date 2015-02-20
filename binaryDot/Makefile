ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
endif


	include 
# this could be conditional as well. 

# Default make rule to use
all: CFGdot


# Compile the CFGdot using the file CFGdot.C
CFGdot.lo:
	libtool --mode=compile g++ $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF)  -c -o CFGdot.lo CFGdot.C


CFGdot: CFGdot.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o CFGdot CFGdot.lo 

clean:
	rm -r .libs
	rm CFGdot
	rm CFGdot.lo
	rm CFGdot.o


