#!/bin/bash

intermediate=piyo.s
binary=a.out
test_flag=0

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
  echo "USAGE: $0 {{input_assembly}} {{test_flag}}" 1>&2
  exit 1
fi

if [ $# = 2 ]; then
  if [ $2 = "true" ] || [ $2 = "1" ]; then
    test_flag=1
  fi
fi

make && python preassemble.py $1 $intermediate  && ./assemble $intermediate $binary && rlwrap ./sim $binary label.txt inst.txt out.txt $test_flag
