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

# Overall counters for modules
n_modules_passed=0
n_modules_failed=0
declare -a failed_module_names=() # Use declare -a for explicit array

MSG_STATUS_OK="[       OK ]"
MSG_STATUS_FAILED="[  FAILED  ]"
MSG_STATUS_RUN="[ RUN      ]"
MSG_PASSED="[  PASSED  ]"
MSG_DASH="[----------]"
MSG_EQUALS="[==========]"

function cl_yellow() {
        printf "${YELLOW}$1${RESET}"
}

function cl_red() {
        printf "${RED}$1${RESET}"
}

function cl_green() {
        printf "${GREEN}$1${RESET}"
}

function pr_ok() {
        printf "$(cl_green "$MSG_STATUS_OK")"
}

function pr_failed() {
        printf "$(cl_red "$MSG_STATUS_FAILED")"
}

function pr_run() {
        printf "$(cl_yellow "$MSG_STATUS_RUN")"
}

function pr_passed() {
        printf "$(cl_green "$MSG_PASSED")"
}

function test_case_run_msg() {
        current_module="$1"
        printf "$(cl_green "$MSG_DASH") Running tests from %s\n" "$current_module"
}

# Keep these functions for formatting individual test case results
function test_ok_msg() {
    local module=$1 # Use local for function args
    local case=$2
    local time=$3
    # Add GREEN color to the [ OK ] line
    printf "$(pr_run) %s.%s\n$(pr_ok) %s.%s (%s ms)\n"\
         "$module" "$case" "$module" "$case" "$time"
}

function test_fail_msg() {
    local module=$1
    local case=$2
    local time=$3
    # Add RED color to the [ FAILED ] line
    printf "$(pr_run) %s.%s\n$(pr_failed) %s.%s (%s ms)\n\
        " "$module" "$case" "$module" "$case" "$time"
}

# Helper function to print module footer and update counts
# Takes the module name and its failure status (true/false) as arguments
function print_module_footer() {
    local module_name="$1"
    local module_failed="$2"

    # Only print if a module name was actually provided (handles edge cases)
    if [[ -n "$module_name" ]]; then
        if [[ "$module_failed" == "true" ]]; then
            printf "$MSG_DASH Tests from %s finished. ($(cl_red 'FAILED'))\n" "$module_name"
            # Increment overall module failure count
            n_modules_failed=$((n_modules_failed + 1))
            # Store the name of the failed module
            failed_module_names+=("$module_name")
        else
            # Add GREEN color to the finished message for passed modules
            printf "$MSG_DASH Tests from %s finished. ($(cl_green 'PASSED'))\n" "$module_name"
            # Increment overall module pass count
            n_modules_passed=$((n_modules_passed + 1))
        fi
        printf "\n" # Add blank line after module footer
    fi
}

printf "$MSG_EQUALS Running tests...\n"

shopt -s globstar

# Iterate through each executable file
for file in **/*.exe; do
    file_path=$(realpath "$file") # Use a distinct variable name
    file_dir=$(dirname "$file_path")

    # --- State variables for processing the output of the current executable ---
    current_module=""           # Tracks the module name of the line being processed
    current_module_failed="false" # Tracks if any test case failed within the current_module
    # ---

    # Change to the executable's directory to run it
    cd "$file_dir"
    # Capture the entire output (colon-separated lines) from the executable
    summary_output=$($file_path)
    # Change back to the main test directory
    cd "$test_dir"

    # Process the captured output line by line
    while IFS= read -r line ; do
        # Remove potential trailing CR from the line
        line=${line%$'\r'}
        # Check if the line matches the expected format
        if [[ "$line" == *":"*":"*":"* ]]; then
            # Parse the line using the colon delimiter
            IFS=':' read -r module_name case_name result time <<< "$line"
            # Remove potential trailing CR from the time variable
            time=${time%$'\r'}

            # --- Module Transition Logic ---
        if [[ "$module_name" != "$current_module" ]]; then
                print_module_footer "$current_module" "$current_module_failed"
                current_module="$module_name"
                current_module_failed="false"
                test_case_run_msg "$current_module"
            fi
            # --- End Module Transition Logic ---

            # --- Process Current Test Case ---
            if [[ "$result" == "true" ]]; then
                test_ok_msg "$module_name" "$case_name" "$time"
            elif [[ "$result" == "false" ]]; then
                test_fail_msg "$module_name" "$case_name" "$time"
                current_module_failed="true"
            fi
            # --- End Process Current Test Case ---
        fi
    done <<< "$summary_output" # Feed the captured output to the while loop

    if [[ "$current_module" == "" ]]; then
        rel_path=$(realpath --relative-to="$test_dir" "$file_dir")
        current_module=$(echo "$rel_path" | sed -E 's|^test/||; s|/|_|g')
        current_module_failed="true"
        test_case_run_msg "$current_module"
    fi

    # --- Footer for the Last Module ---
    print_module_footer "$current_module" "$current_module_failed"
    # --- End Footer for the Last Module ---

done # End of loop through *.exe files

shopt -u globstar # Turn off globstar

# --- Final Summary ---
n_total_modules=$((n_modules_passed + n_modules_failed))
# Add GREEN color to the overall status line
printf "$(cl_green $MSG_EQUALS) Finished running %d test modules.\n" "$n_total_modules"
# Add GREEN color to the PASSED summary line
printf "$(pr_passed) %d test modules.\n" "$n_modules_passed"

if [ $n_modules_failed != 0 ]; then
    unique_failed_names=($(printf "%s\n" "${failed_module_names[@]}" | sort -u))
    # Add RED color to the FAILED summary line
    printf "$(pr_failed) %d test modules, listed below:\n" "${#unique_failed_names[@]}"
    for failed_name in "${unique_failed_names[@]}"; do
        printf "$(pr_failed) %s\n" "$failed_name"
    done
    printf "\n%d FAILED TEST MODULES\n" "${#unique_failed_names[@]}"
fi
# --- End Final Summary ---

# Return to the starting directory
cd "$start_dir"

# Exit with non-zero status if any modules failed
if [ $n_modules_failed != 0 ]; then
    exit 1
else
    exit 0
fi
