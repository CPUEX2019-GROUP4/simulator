#!/bin/bash
#make && python preassemble.py && ./assemble piyo.s && rlwrap ./sim a.out label.txt inst.txt
intermediate=piyo.s
binary=a.out

if [ $# != 1 ]; then
  echo "USAGE: $0 {{input_assembly}}" 1>&2
  exit 1
fi


make && python preassemble.py $1 $intermediate  && ./assemble $intermediate $binary && rlwrap ./sim $binary label.txt inst.txt out.txt
