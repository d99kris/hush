Hush
====

| **Linux + Mac** |
|-----------------|
| [![Build status](https://travis-ci.com/d99kris/hush.svg?branch=master)](https://travis-ci.com/d99kris/hush) |

Hush suppresses stdout for commands executed successfully. It executes
specified program and shows its output only after the program has exited.
If the exit status is zero, only stderr from the program is outputted.
If the exit status is non-zero both stderr and stdout are outputted.

Example Usage
=============

    $ hush make

Supported Platforms
===================
Hush should work on Linux and macOS.

Installation
============
Pre-requisites (Ubuntu):

    sudo apt install git cmake build-essential

Download the source code:

    git clone https://github.com/d99kris/hush && cd hush

Generate Makefile and build:

    mkdir -p build && cd build && cmake .. && make -s

Optionally run tests:

    ctest --output-on-failure

Optionally install in system:

    sudo make install

Usage
=====

General usage syntax:

    hush PROG [ARGS..]

Technical Details
=================
Hush executes specified program using faketty (preserving color output) and
captures stdout, stderr and a combination of them both. Depending on process
exit status, the appropriate output is propagated to the user / shell.

License
=======
Hush is distributed under the BSD 3-Clause license. See LICENSE file.

Keywords
========
hush, linux, macos, os x, silenced execution, suppressing stdout upon successful exit status.
