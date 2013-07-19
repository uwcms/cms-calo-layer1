SPI Stream Test Suite
=====================

This folder contains minimally complex unit tests for various functionality.
It is intended to be compiled outside of Xilinx to host-native binaries.
Xilinx should be configured to ignore this folder.

Tests for a given source file in ``../src`` must use the same name in this
folder.  In other words, ``src/tests/circular_buffer.c`` contains tests for 
``src/circular_buffer.c``.   This is then compiled into a test binary
``test_circular_buffer.exe`` which runs the unit tests.

Each test ``.c`` should define the following objects:

```c
int tests_run; // Don't know why can't have this in minunit.c, but linker is mad
char * all_tests(void);
```

``all_tests`` should run all the tests in that ``.c`` file.
