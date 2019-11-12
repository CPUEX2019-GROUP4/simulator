#!/bin/bash

intermediate=piyo.s
binary=a.out
test_flag=0
exe=./sim

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "USAGE: $0 {{input_assembly}} {{input_file}} {{test_flag}}" 1>&2
  echo "only test_flag is optional. if not specified, it is set false" 1>&2
  exit 1
fi

if [ $# = 3 ]; then
  if [ $3 = "true" ] || [ $3 = "1" ]; then
    exe=./run
  fi
  if [ $3 = "label" ]; then
    exe=./run_label
  fi
fi

make &&
time -p {
./preassemble $1 $intermediate && ./assemble $intermediate $binary && $exe $binary label.txt inst.txt out.txt $test_flag $2
}
