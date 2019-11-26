#!/bin/bash

intermediate=piyo.s
binary=a.out
test_flag=0
exe=./sim
pre=:
post=:

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "USAGE: $0 {{input_assembly}} {{input_file}} {{test_flag}}" 1>&2
  echo "only test_flag is optional. if not specified, it is set false" 1>&2
  exit 1
fi

execute () {
  ./preassemble $1 $intermediate && ./assemble $intermediate $binary && $3 $binary label.txt inst.txt out.txt $test_flag $2
}

if [ $# = 3 ]; then
  if [ $3 = "true" ] || [ $3 = "1" ] || [ $3 = "run" ]; then
    make && time execute $1 $2 ./run
  elif [ $3 = "false" ] || [ $3 = "0" ] || [ $3 = "sim" ]; then
    make && execute $1 $2 ./sim
  elif [ $3 = "label" ]; then
    make && time execute $1 $2 ./run_label
  else
    # assume $3 is an instruction to inspect count
    echo -e "\x1b[1mcounting mode!!\x1b[0m"
    cp run_label.cc run_label.cc.bak    # back up
    sed -i "s&// $3\$&// $3\ninst_counter++;&g" run_label.cc

    diff=$(diff run_label.cc run_label.cc.bak)
    if [[ $diff = "" ]]; then
      echo "Invalid operand. No such opcode as: $3. Abort."
      exit 1
    fi

    make run_label
    mv run_label.cc.bak run_label.cc ;  # restore
    make
    execute $1 $2 ./run_label
    echo -e "counter was for \x1b[1m$3\x1b[0m"
  fi
fi

# if mode is not given, assume SIM
if [ $# -lt 3 ]; then
  make && execute $1 $2 ./sim
fi
