#!/bin/bash

file="$1"

# Check if the file name was provided as an argument
if [ -z "$file" ]; then
  echo "Error: Please provide the header file name as an argument."
  exit 1
fi

# Extract the current REV field from the header file
rev=$(grep "#define REV" $file | awk '{print $3}')

# Increment the REV field by 1
new_rev=$((rev + 1))

# Replace the current REV field with the incremented value
sed -i "s/#define REV $rev/#define REV $new_rev/g" $file
