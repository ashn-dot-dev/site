#!/bin/sh
# The default version of g++ shipping with Ubuntu 16.04 (g++-7) will produce a
# compilation error since libstdc++fs is still marked as experimental for C++17
# code.
# Running:
#   $ sudo apt-get install g++-8
# will give you g++-8, which does support the C++17 filesystem API.
g++-8 -o dvd -std=c++17 main.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs
