#!/bin/bash

# The input JSON file
input_file="utilities/version/version.json"
tmp_file="utilities/version/version.json.tmp"

### Bump the version ###
# Increment the rev field in the version.json file
jq ".EMBEDDED_FLASH_REV += 1" $input_file > $tmp_file
mv $tmp_file $input_file

### C ###

# The output header file
c_out_file="src/emb_ext_flash_version.h"

# Parse the JSON file to extract the constants
major=$(cat $input_file | jq -r '.EMBEDDED_FLASH_MAJOR')
minor=$(cat $input_file | jq -r '.EMBEDDED_FLASH_MINOR')
rev=$(cat $input_file | jq -r '.EMBEDDED_FLASH_REV')

# Write the constants to the header file
echo "#ifndef EMB_EXT_FLASH_VERSION_H" > $c_out_file
echo "#define EMB_EXT_FLASH_VERSION_H" >> $c_out_file
echo "" >> $c_out_file
echo "#define EMBEDDED_FLASH_MAJOR $major" >> $c_out_file
echo "#define EMBEDDED_FLASH_MINOR $minor" >> $c_out_file
echo "#define EMBEDDED_FLASH_REV $rev" >> $c_out_file
echo "" >> $c_out_file
echo "#endif /* EMB_EXT_FLASH_VERSION_H */" >> $c_out_file

git add $input_file
git add $c_out_file
