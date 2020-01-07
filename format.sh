#!/bin/bash

clang-format -i include/flute/*.h

clang-format -i src/flute/*.cc
clang-format -i src/flute/*.h
# clang-format -i src/flute/detail/*.cc
clang-format -i src/flute/detail/*.h
clang-format -i examples/gameserver/login/*.cc