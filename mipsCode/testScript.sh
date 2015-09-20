#!/bin/bash

# This script is intended to be used to run all the test binaries
# through the framework.

BINS=./binaries/*

echo -e "Testing all binaries in naive mode then optimized mode.\n"
#how about adding opt or naive as variable and the same with debugging.
for bin in $BINS
do
    FUNC=$(basename $bin .out)
    echo "Testing naive mode on ${bin##*/} and transforming function ${FUNC}" 
    ../tmr/userRewriter.out "$bin" "$FUNC" "0" > /dev/null 2>&1
    RETVAL=$?
    if [ ! "$RETVAL" -eq "0" ]
    then 
        # the Return value is a failure value.
        echo -e "Naive transform on $bin, function $FUNC, failed.\n" 
    else 
        echo -e "Naive transformation on ${FUNC} in binary ${bin##*/} successful\n" 
    fi
    # Test the optimized version for the same binary
    echo "Testing optimized mode on ${bin##*/} and transforming function ${FUNC}" 
    ../tmr/userRewriter.out "$bin" "$FUNC" "1" > /dev/null 2>&1
    if [ ! "$RETVAL" -eq "0" ]
    then 
        # the Return value is a failure value.
        echo -e "Opt transform on function $FUNC in binary $bin failed.\n"
        exit
    else
        echo -e "Opt transformation on ${FUNC} in binary ${bin##*/} successful\n" 
    fi
done
echo -e "Tests finished\n"


