include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars

ROSE_SOURCE_DIR = /home/david/Github/ROSE/edg4x-rose/tutorial

# Default make rule to use
all: exampleTranslator
	@echo "End of all rule"
	# @if [ x$${ROSE_IN_BUILD_TREE:+present} = xpresent ]; then echo "ROSE_IN_BUILD_TREE should not be set" >&2; exit 1; fi

# Example suffix rule for more experienced makefile users
# .C.o:
#	g++ -c -I$(ROSE_INCLUDE_DIR) -o $@ $(@:.o=.C)

# Compile the exampleTranslator using the file identityTranslator.C
exampleTranslator.lo:
	libtool --mode=compile g++ $(ROSE_INCLUDES) -I/home/david/Github/ROSE/edg4x-rose/compiletree -c -o exampleTranslator.lo $(ROSE_SOURCE_DIR)/identityTranslator.C

exampleTranslator: exampleTranslator.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o exampleTranslator exampleTranslator.lo 

# Rule used by make installcheck to verify correctness of installed libraries
#check:
#	./exampleTranslator -c $(ROSE_SOURCE_DIR)/testCode.C


