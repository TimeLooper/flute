#!/bin/bash

cmake -B build -G "Unix Makefiles"
cmake --build build --config Debug --target all -- -j 4
