# About
This is the unit test module for the C implementation of the embedded external flash memory. We leverage the google test framework for compartmentalized unit testing for the interface implementation. For unit testing purposes we have created a simulated flash memory bank that occupies space in RAM. You can read more about the google test framework here: https://github.com/google/googletest.

# Dependencies
You will only need dependencies for the google test framework to run these unit tests - all release versions are passing unit tests.

# Run
To run the unit test framework run the following commands out of this directory:
```
rm -rf build
cmake -S . -B build
cmake --build build
./build/protocol_test
```
