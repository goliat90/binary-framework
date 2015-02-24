ifeq ($(USER),david) 
    include /home/david/Github/ROSE/edg4x-rose/compiletree/Makefile_vars
	ROSE_SOURCE_DIR = /home/david/Github/ROSE/edg4x-rose/tutorial
	PATHDIFF = david/Github/ROSE/edg4x-rose/compiletree
else
    include /home/almd/Github/edg4x-rose/Compiletree/Makefile_vars
	ROSE_SOURCE_DIR = /home/almd/Github/edg4x-rose/tutorial
	PATHDIFF = almd/Github/edg4x-rose/Compiletree
endif


	include 
# this could be conditional as well. 

# Default make rule to use
all: exampleTranslator


# Compile the exampleTranslator using the file identityTranslator.C
exampleTranslator.lo:
	libtool --mode=compile g++ $(ROSE_INCLUDES) -I/home/$(PATHDIFF) -c -o exampleTranslator.lo $(ROSE_SOURCE_DIR)/identityTranslator.C


exampleTranslator: exampleTranslator.lo
	libtool --mode=link g++ $(ROSE_LIBS) -o exampleTranslator exampleTranslator.lo 

clean:
	rm -rf .libs
	rm -f exampleTranslator*

