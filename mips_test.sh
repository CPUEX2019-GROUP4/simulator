#!/bin/bash

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
  echo "USAGE: $0 {{input_the_name_of_program}} {{test_flag}}" 1>&2
  exit 1
fi

DIR_COMPILER=../compiler

cd $DIR_COMPILER
cd min-caml
make min-caml
make test/$1.s
make test/$1.ans
#vim test/$1.s
cat test/$1.s libmincaml.S > ../../simulator/sim/mips_test.s
cd ../../simulator/sim/
#./test.sh mips_test.s
if [ $# = 2 ]; then
  ./test.sh mips_test.s $2
else
  ./test.sh mips_test.s
fi

echo "--- out.txt ---"
cat out.txt
echo ""

echo "--- answer ---"
cat ../$DIR_COMPILER/min-caml/test/$1.ans
echo ""
