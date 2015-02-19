ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	ROSE_SOURCE_DIR = /home/david/Github/ROSE/edg4x-rose/tutorial
else
# add path for other computer here. 
endif

# this could be conditional as well. 

# Default make rule to use
all: exampleTranslator


# Compile the exampleTranslator using the file identityTranslator.C
exampleTranslator.lo:
	libtool --mode=compile g++ $(ROSE_INCLUDES) -I/home/david/Github/ROSE/edg4x-rose/compiletree -c -o exampleTranslator.lo $(ROSE_SOURCE_DIR)/identityTranslator.C


exampleTranslator: exampleTranslator.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o exampleTranslator exampleTranslator.lo 



