#!/bin/bash

find ./src -name "*.*"|xargs clang-format -i
find ./include -name "*.*"|xargs clang-format -i
