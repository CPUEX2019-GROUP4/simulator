#!/bin/sh
make && python preassemble.py && ./assemble piyo.s && rlwrap ./sim a.out label.txt inst.txt
