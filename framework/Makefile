ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
endif

SRCDIR = ./src
OBJDIR = ./objectfiles

# testing adding more folders to vpath
VPATH = src:include:objectfiles

# Rules that do not depend on a file.
.PHONY: clean all linkExecutable

all: libConvenience.la

graphDAG.lo: graphDAG.cpp graphDAG.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/graphDAG.lo \
	$(SRCDIR)/graphDAG.cpp

listScheduler.lo: listScheduler.cpp listScheduler.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/listScheduler.lo \
	$(SRCDIR)/listScheduler.cpp

liveVariableAnalysis.lo: liveVariableAnalysis.cpp liveVariableAnalysis.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/liveVariableAnalysis.lo \
	$(SRCDIR)/liveVariableAnalysis.cpp

linearScan.lo: linearScan.cpp linearScan.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/linearScan.lo \
	$(SRCDIR)/linearScan.cpp

naiveTransform.lo: naiveTransform.cpp naiveTransform.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/naiveTransform.lo \
	$(SRCDIR)/naiveTransform.cpp

cfgHandler.lo: cfgHandler.cpp cfgHandler.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/cfgHandler.lo \
	$(SRCDIR)/cfgHandler.cpp

binaryDebug.lo: binaryDebug.cpp binaryDebug.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/binaryDebug.lo \
	$(SRCDIR)/binaryDebug.cpp

mipsISA.lo: mipsISA.cpp mipsISA.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/mipsISA.lo \
	$(SRCDIR)/mipsISA.cpp

symbolicRegisters.lo: symbolicRegisters.cpp symbolicRegisters.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/symbolicRegisters.lo \
	$(SRCDIR)/symbolicRegisters.cpp

binaryRewriter.lo: binaryRewriter.cpp binaryRewriter.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/binaryRewriter.lo \
	$(SRCDIR)/binaryRewriter.cpp

test.lo: test.cpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I./include -I/home/$(PATHDIFF) -c -o $(OBJDIR)/test.lo \
	$(SRCDIR)/test.cpp


# Compile a library object file
libConvenience.la: binaryRewriter.lo symbolicRegisters.lo mipsISA.lo binaryDebug.lo cfgHandler.lo naiveTransform.lo \
		linearScan.lo liveVariableAnalysis.lo listScheduler.lo graphDAG.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o libConvenience.la $(OBJDIR)/*.lo

# Executable rule that can be used in case
linkExecutable: binaryRewriter.lo test.lo symbolicRegisters.lo mipsISA.lo binaryDebug.lo cfgHandler.lo naiveTransform.lo \
		linearScan.lo liveVariableAnalysis.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o test.out $(OBJDIR)/*.lo


clean:
	rm -rf .libs
	rm -f $(OBJDIR)/*
	rm -f test.out
	rm -f libConvenience.la

