#!/bin/bash

YELLOW='\033[0;33m'
RESET='\033[0m'

# Store the current directory
start_dir=$(pwd)

# Change directory to the specific relative path
cd build/test

echo "[==========] Running MC test suite"
n_crash=0

shopt -s globstar 

for file in **/*.exe; do

        "./$file"
        exit_code=$?
        if [ $exit_code != 0 ]; then
                n_crash=$((n_crash + 1))
        fi        
done

shopt -u globstar 

echo "[==========] 1 tests from 1 test case ran."
if [ $n_crash != 0 ]; then
        echo "$n_crash CRASHED TESTS"
fi

# Return to the starting directory
cd "$start_dir"
exit 0
