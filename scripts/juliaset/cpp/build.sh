#!/bin/bash
mkdir -p build
cd build
cmake ..
make
make test
cd CMakeFiles/test_juliaset.dir
lcov -d . -c -o coverage.info
lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info
genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
cd ../../..
mkdir -p docs
cd docs
doxygen -g
patch < ../patch/Doxyfile.diff
doxygen
cd ..
find . -maxdepth 1 \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs --null -I '{}' sh -c 'clang-format "$1" > "$1".new' -- '{}'
clang-tidy *.cpp -checks=perf\* -- -std=c++20 -I . -I build/googletest-src/googletest/include
