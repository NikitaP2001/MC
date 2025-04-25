#!/bin/bash

YELLOW='\033[0;33m'
GREEN='\033[0;32m'
RED='\033[1;31m'
RESET='\033[0m'

start_dir=$(pwd)

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

function test_ok_msg() {
        local module=$1
        local case=$2
        local time=$3
        printf "$(pr_run) %s.%s\n$(pr_ok) %s.%s (%s ms)\n"\
         "$module" "$case" "$module" "$case" "$time"
}

function test_fail_msg() {
        local module=$1
        local case=$2
        local time=$3
        printf "$(pr_run) %s.%s\n$(pr_failed) %s.%s (%s ms)\n\
        " "$module" "$case" "$module" "$case" "$time"
}

# Helper function to print module footer and update counts
# Takes the module name and its failure status (true/false) as arguments
function print_module_footer() {
        local module_name="$1"
        local module_failed="$2"

        if [[ -n "$module_name" ]]; then
                if [[ "$module_failed" == "true" ]]; then
                        printf "$MSG_DASH Tests from %s finished. ($(cl_red 'FAILED'))\n" "$module_name"
                        n_modules_failed=$((n_modules_failed + 1))
                        failed_module_names+=("$module_name")
                else
                        printf "$MSG_DASH Tests from %s finished. ($(cl_green 'PASSED'))\n" "$module_name"
                        n_modules_passed=$((n_modules_passed + 1))
                fi
        fi
}

printf "$MSG_EQUALS Running tests...\n"

shopt -s globstar

# Iterate through each executable file
for file in **/*.exe; do
        file_path=$(realpath "$file")
        file_dir=$(dirname "$file_path")

        current_module=""           
        current_module_failed="false"
	test_case_buffer=""

        cd "$file_dir"
        summary_output=$($file_path)
        cd "$test_dir"

        while IFS= read -r line ; do
                line=${line%$'\r'}
                if [[ "$line" == *":"*":"*":"* ]]; then
                        IFS=':' read -r module_name case_name result time <<< "$line"
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
                                test_case_buffer+=$(test_ok_msg "$module_name" "$case_name" "$time")"\n"
                        elif [[ "$result" == "false" ]]; then
                                test_case_buffer+=$(test_fail_msg "$module_name" "$case_name" "$time")"\n"
                                current_module_failed="true"
                        fi
                        # --- End Process Current Test Case ---
                fi
        done <<< "$summary_output"

        if [[ "$current_module" == "" ]]; then
                rel_path=$(realpath --relative-to="$test_dir" "$file_dir")
                current_module=$(echo "$rel_path" | sed -E 's|^test/||; s|/|_|g')
                current_module_failed="true"
                test_case_run_msg "$current_module"
        fi

	if [[ -n "$test_case_buffer" && "$current_module_failed" == "true" ]]; then
		echo -e "$test_case_buffer"   
	fi

        print_module_footer "$current_module" "$current_module_failed"

done

shopt -u globstar

# --- Final Summary ---
n_total_modules=$((n_modules_passed + n_modules_failed))
printf "$(cl_green $MSG_EQUALS) Finished running %d test modules.\n" "$n_total_modules"
printf "$(pr_passed) %d test modules.\n" "$n_modules_passed"

if [ $n_modules_failed != 0 ]; then
        unique_failed_names=($(printf "%s\n" "${failed_module_names[@]}" | sort -u))
        printf "$(pr_failed) %d test modules, listed below:\n" "${#unique_failed_names[@]}"
        for failed_name in "${unique_failed_names[@]}"; do
                printf "$(pr_failed) %s\n" "$failed_name"
        done
        printf "\n%d FAILED TEST MODULES\n" "${#unique_failed_names[@]}"
fi
# --- End Final Summary ---

cd "$start_dir"

if [ $n_modules_failed != 0 ]; then
        exit 1
else
        exit 0
fi