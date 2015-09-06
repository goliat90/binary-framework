#!/bin/bash

# This script is used to generate graphs of all
# the test binaries.

BINS=./binaries/*

echo "Building graphs for binaries."
#how about adding opt or naive as variable and the same with debugging.
for bin in $BINS
do
    echo "Graph for ${bin##*/}." 
    # Get the function name from the binary.
    FUNC=$(basename $bin .out)
    # Call the graph writer.
    ../binaryDot/CFGdot.out "$bin" "$FUNC" "0"
    # Generate a png from the dot
    dot "-Tpng" "${FUNC}0.dot" "-o${FUNC}0.png"
    # Clean up by removing the dot file.
    rm "${FUNC}0.dot"
done
