#!/bin/bash

intermediate=piyo.s
binary=a.out
test_flag=0

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "USAGE: $0 {{input_assembly}} {{input_file}} {{test_flag}}" 1>&2
  echo "only test_flag is optional. if not specified, it is set false" 1>&2
  exit 1
fi

if [ $# = 3 ]; then
  if [ $3 = "true" ] || [ $3 = "1" ]; then
    test_flag=1
  fi
fi

#make && python preassemble.py $1 $intermediate  && ./assemble $intermediate $binary && rlwrap ./sim $binary label.txt inst.txt out.txt $test_flag $2
#make && ./preassemble $1 && python preassemble.py $1 $intermediate  && ./assemble $intermediate $binary && rlwrap ./sim $binary label.txt inst.txt out.txt $test_flag $2
#make && ./preassemble $1 $intermediate  && ./assemble $intermediate $binary && rlwrap ./sim $binary label.txt inst.txt out.txt $test_flag $2
make && ./preassemble $1 $intermediate && ./assemble $intermediate $binary && rlwrap ./sim $binary label.txt inst.txt out.txt $test_flag $2
