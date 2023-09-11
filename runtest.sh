#!/bin/bash

# Store the current directory
start_dir=$(pwd)

# Change directory to the specific relative path
cd build/test

find . -name "*.exe" -exec {} \;

read -rp "Press Enter to continue..."

# Return to the starting directory
cd "$start_dir"
exit 0