#!/bin/bash

# Trigger all your test cases with this script

#! /usr/bin/env sh
echo "##########################"
echo "### Running e2e tests! ###"
echo "##########################"
count=0 # number of test cases run so far

# Assume all `.in` and `.out` files are located in a separate `tests` directory


for test in tests_folder/*.in; do
    name=$(basename $test .in)
    expected=tests_folder/$name.out
    args=tests_folder/$name.in

    echo running $test
    xargs -a $args -I % ./tests % 2>&1 | diff - $expected || echo "Test $name: failed!"


    count=$((count+1))
done

echo "Finished running $count tests!"