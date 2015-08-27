# This make file will build cfg dot writer, framework library,
# test binaries and the tmr program.
# It also has clean for the folders.

all: buildAll

buildAll:
	$(MAKE) -C ./binaryDot
	$(MAKE) -C ./mipsCode
	$(MAKE) -C ./framework
	$(MAKE) -C ./tmr


clean:
	$(MAKE) -C ./binaryDot clean
	$(MAKE) -C ./mipsCode clean
	$(MAKE) -C ./framework clean
	$(MAKE) -C ./tmr clean

