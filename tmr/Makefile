ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
	LIBINCDIR = /home/david/Github/ROSE/binary-framework/framework/include
	LIBSRCDIR = /home/david/Github/ROSE/binary-framework/framework/src
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
	LIBINCDIR = /home/almd/Github/binary-framework/framework/include
	LIBSRCDIR = /home/almd/Github/binary-framework/framework/src
endif

SRCDIR = ./src

all: linking 

framework.lo:
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I/home/$(PATHDIFF) \
	-c -o binaryRewriter.lo $(LIBSRCDIR)/binaryRewriter.cpp

userFramework.lo:
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I./include -I/home/$(PATHDIFF) \
	-c -o userFramework.lo $(SRCDIR)/userFramework.cpp

linking: framework.lo userFramework.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o userRewriter.out userFramework.lo binaryRewriter.lo

	

clean:
	rm -rf .libs
	rm -f userFramework.lo
	rm -f userFramework.o
	rm -f binaryRewriter.lo
	rm -f binaryRewriter.o
	rm -f userRewriter.out


