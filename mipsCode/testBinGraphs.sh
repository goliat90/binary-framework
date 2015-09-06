#!/bin/bash

# This script is used to generate graphs of all
# the test binaries.

BINS=./binaries/*


for bin in ./binaries/*



echo "Testing all binaries."
#how about adding opt or naive as variable and the same with debugging.
for bin in $BINS
do
    echo "Testing ${bin##*/}." 
    FUNC=$(basename $bin .out)
    ../tmr/userRewriter.out $bin $FUNC 0 || exit

# catch the failure and continue with a test.
done
