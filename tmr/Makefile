ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
	LIBINCDIR = /home/david/Github/binary-framework/framework/include
	LIBSRCDIR = /home/david/Github/binary-framework/framework/src
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
	LIBINCDIR = /home/almd/Github/binary-framework/framework/include
	LIBSRCDIR = /home/almd/Github/binary-framework/framework/src
endif

SRCDIR = ./src

VPATH = $(LIBSRCDIR):$(LIBINCDIR):src:include

all: linking 

naiveTransform.lo: naiveTransform.cpp naiveTransform.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I/home/$(PATHDIFF) -c -o naiveTransform.lo \
	$(LIBSRCDIR)/naiveTransform.cpp

cfgHandler.lo: cfgHandler.cpp cfgHandler.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I/home/$(PATHDIFF) -c -o cfgHandler.lo \
	$(LIBSRCDIR)/cfgHandler.cpp

binaryDebug.lo: binaryDebug.cpp binaryDebug.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I/home/$(PATHDIFF) -c -o binaryDebug.lo \
	$(LIBSRCDIR)/binaryDebug.cpp

mipsISA.lo: mipsISA.cpp mipsISA.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I/home/$(PATHDIFF) -c -o mipsISA.lo \
	$(LIBSRCDIR)/mipsISA.cpp

symbolicRegisters.lo: symbolicRegisters.cpp symbolicRegisters.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I/home/$(PATHDIFF) -c -o symbolicRegisters.lo \
	$(LIBSRCDIR)/symbolicRegisters.cpp

framework.lo: binaryRewriter.cpp binaryRewriter.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I/home/$(PATHDIFF) -c -o binaryRewriter.lo \
	$(LIBSRCDIR)/binaryRewriter.cpp

userFramework.lo: userFramework.cpp userFramework.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I./include -I/home/$(PATHDIFF) \
	-c -o userFramework.lo $(SRCDIR)/userFramework.cpp

linking: framework.lo userFramework.lo symbolicRegisters.lo mipsISA.lo binaryDebug.lo cfgHandler.lo naiveTransform.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o userRewriter.out userFramework.lo binaryRewriter.lo symbolicRegisters.lo \
	mipsISA.lo binaryDebug.lo cfgHandler.lo naiveTransform.lo

debug:
	libtool --mode=execute gdb --args ./userRewriter.out littleend.out

clean:
	rm -rf .libs
	rm -f cfgHandler.lo
	rm -f cfgHandler.o
	rm -f binaryDebug.lo
	rm -f binaryDebug.o
	rm -f mipsISA.lo
	rm -f mipsISA.o
	rm -f symbolicRegisters.lo
	rm -f symbolicRegisters.o
	rm -f binaryRewriter.lo
	rm -f binaryRewriter.o
	rm -f userFramework.lo
	rm -f userFramework.o
	rm -f naiveTransform.lo
	rm -f naiveTransform.o
	rm -f userRewriter.out


