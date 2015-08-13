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
FRAMEWORKLIBDIR = ../framework
OBJDIR = ./objectfiles

VPATH = $(LIBSRCDIR):$(LIBINCDIR):$(FRAMEWORKLIBDIR):src:include:objectfiles

#phony rules
.PHONY: all buildLibrary debug test clean

all: userRewriter.out

userFramework.lo: userFramework.cpp userFramework.hpp
	libtool --mode=compile g++ -g $(ROSE_INCLUDES) -I$(LIBINCDIR) -I./include -I/home/$(PATHDIFF) \
	-c -o $(OBJDIR)/userFramework.lo $(SRCDIR)/userFramework.cpp

# rule to include framework in compilation.
buildLibrary:
	@echo -e "\n Compiling framework library file. \n"
	$(MAKE) -C $(FRAMEWORKLIBDIR)
	@echo -e "\n Framework library file compiled. \n"

# compile executable by using the library object file
userRewriter.out: buildLibrary userFramework.lo 
	libtool --mode=link g++ $(ROSE_LIBS) -o userRewriter.out $(OBJDIR)/*.lo $(FRAMEWORKLIBDIR)/libConvenience.la


simpletestall: simpletestnaive simpletestopt

simpletestnaive:
	./userRewriter.out ../mipsCode/simpleAdd.out simpleAdditionTMR 0
simpletestopt:
	./userRewriter.out ../mipsCode/simpleAdd.out simpleAdditionTMR 1 
simpledebug:
	libtool --mode=execute gdb --args ./userRewriter.out ../mipsCode/simpleAdd.out simpeAdditionTMR 


advancedtestall: advancedtestnaive advancedtestopt

advancedtestnaive:
	./userRewriter.out ../mipsCode/advancedAdd.out advancedAdditionTMR 0
advancedtestopt:
	./userRewriter.out ../mipsCode/advancedAdd.out advancedAdditionTMR 1 
advanceddebug:
	libtool --mode=execute gdb --args ./userRewriter.out ../mipsCode/advancedAdd.out advancedAdditionTMR


clean:
	rm -rf .libs
	rm -f $(OBJDIR)/*
	rm -f userRewriter.out
	$(MAKE) -C $(FRAMEWORKLIBDIR) clean


