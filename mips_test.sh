#!/bin/bash

if [ $# != 1 ]; then
  echo "USAGE: $0 {{input_the_name_of_program}}" 1>&2
  exit 1
fi

DIR_COMPILER=../compiler

cd $DIR_COMPILER
cd min-caml
make min-caml
make test/$1.s
make test/$1.ans
vim test/$1.s
cp test/$1.s ../../simulator/sim/mips_test.s
cd ../../simulator/sim/
./test.sh mips_test.s
