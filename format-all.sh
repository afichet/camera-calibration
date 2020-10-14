#!/bin/sh

find lib -regex '.*\.\(c\|cpp\|h\)' -exec sed -i "s/#pragma omp/\\/\\/#pragma omp/g" {} \;
find lib -regex '.*\.\(c\|cpp\|h\)' -exec clang-format -style=file -i {} \;
find lib -regex '.*\.\(c\|cpp\|h\)' -exec sed -i "s/\\/\\/ *#pragma omp/#pragma omp/g" {} \;


find apps -regex '.*\.\(c\|cpp\|h\)' -exec sed -i "s/#pragma omp/\\/\\/#pragma omp/g" {} \;
find apps -regex '.*\.\(c\|cpp\|h\)' -exec clang-format -style=file -i {} \;
find apps -regex '.*\.\(c\|cpp\|h\)' -exec sed -i "s/\\/\\/ *#pragma omp/#pragma omp/g" {} \;

