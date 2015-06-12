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

VPATH = $(LIBSRCDIR):$(LIBINCDIR):src:include:objectfiles

#phony rules
.PHONY: all buildLibrary linkingExecutable debug test clean

all: userRewriter.out

# rule to include framework in compilation.
buildLibrary:
	$(MAKE) -C $(FRAMEWORKLIBDIR)

# compile executable by using the library object file
userRewriter.out: buildLibrary userFramework.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o userRewriter.out $(OBJDIR)/*.lo $(FRAMEWORKLIBDIR)/libConvenience.la


debug:
	libtool --mode=execute gdb --args ./userRewriter.out littleend.out

test:
	./userRewriter.out littleend.out

clean:
	rm -rf .libs
	rm -f $(OBJDIR)/*
	rm -f userRewriter.out
	$(MAKE) -C $(FRAMEWORKLIBDIR) clean


