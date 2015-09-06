#!/bin/bash

# This script is intended to be used to run all the test binaries
# through the framework.

BINS=./binaries/*

echo "Testing all binaries."
#how about adding opt or naive as variable and the same with debugging.
for bin in $BINS
do
    FUNC=$(basename $bin .out)
    echo "Testing ${bin##*/} and transforming function ${FUNC}" 
    ../tmr/userRewriter.out $bin $FUNC 0 || exit

# catch the failure and continue with a test.
done
