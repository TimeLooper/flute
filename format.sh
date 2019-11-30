#!/bin/bash

clang-format -i include/flute/*.h

clang-format -i src/flute/*.cpp
clang-format -i src/flute/impl/*.cpp