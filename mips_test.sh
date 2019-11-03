#!/bin/bash

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "USAGE: $0 {{program_name}} {{input_file}} {{test_flag}}" 1>&2
  echo "only test_flag is optional. if not specified, it is set false" 1>&2
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
./test.sh mips_test.s foo.s

echo "--- out.txt ---"
cat out.txt
echo ""

echo "--- answer ---"
cat ../$DIR_COMPILER/min-caml/test/$1.ans
echo ""
