#!/bin/bash

file="$1"

# Check if the file name was provided as an argument
if [ -z "$file" ]; then
  echo "Error: Please provide the header file name as an argument."
  exit 1
fi

# Build the unit test suite
rm -rf $file/build
cmake -S $file -B $file/build
cmake --build $file/build
./$file/build/emb_ext_flash_test

# Check the exit status of the unit tests
if [ $? -ne 0 ]; then
  echo "Unit tests failed, push terminated"
  exit 1
fi

# If all unit tests are passing, allow the push
exit 0