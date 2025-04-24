#!/bin/bash

YELLOW='\033[0;33m'
GREEN='\033[0;32m'
RED='\033[1;31m'
RESET='\033[0m'

# Store the current directory
start_dir=$(pwd)

# Change directory to the specific relative path
test_dir=$(realpath build/test)
cd "$test_dir"

echo "[==========] Running MC test suite"
n_failed=0
n_passed=0

shopt -s globstar 

# TODO: Refactor notes
# Use case_result=$(file) to get the result of the test case
# Parse results in form module_name:case_name:result
# Collect this information, and in case of failure print all the results
# Othervise only print module name and number of passed tests

for file in **/*.exe; do
        file=$(realpath "$file")
        file_dir=$(dirname "$file")

        rel_path=$(realpath --relative-to="$test_dir" "$file_dir")
        test_name=$(echo "$rel_path" | sed -E 's|^test/||; s|/|_|g')

        echo -e "[----------] Starting $test_name test set"

        cd "$file_dir"
        "$file"
        exit_code=$?
        cd "$test_dir"

        if [ $exit_code != 0 ]; then
                n_failed=$((n_failed + 1))
                echo -e "${RED}[==========]${RESET} Test set $test_name FAILED"
        else
                n_passed=$((n_passed + 1))
                echo -e "${GREEN}[----------]${RESET} Test set $test_name OK"
        fi        
done

shopt -u globstar 

n_total=$((n_passed + n_failed))
echo "[==========] Test suite finished, $n_passed / $n_total"
if [ $n_passed != 0 ]; then
        echo -e "$n_passed ${GREEN}PASSED${RESET} TESTS"
fi
if [ $n_failed != 0 ]; then
        echo -e "$n_failed ${RED}FAILED${RESET} TESTS"
fi

# Return to the starting directory
cd "$start_dir"
exit 0
