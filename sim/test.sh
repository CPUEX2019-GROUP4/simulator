#!/bin/sh
make && python preassemble.py && ./assemble piyo.s && ./sim a.out
