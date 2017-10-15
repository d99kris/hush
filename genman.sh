#!/bin/bash

mkdir -p build && \
cd build && \
cmake .. && \
make && \
cd bin && \
help2man -n "fake tty execution" -N -o ../../src/faketty.1 ./faketty && \
help2man -n "suppress stdout for commands executed successfully" -N -o ../../src/hush.1 ./hush && \
cd ../.. && \
rm -rf build

exit ${?}
