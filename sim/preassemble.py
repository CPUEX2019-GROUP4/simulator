#!/usr/bin/env python
# coding: utf-8

# 各命令の行番号と命令番号の対応をとるとともに、ラベルを定数に置換する
# (行番号, 命令番号) というpairを作れば、ラベルは行番号に置き換えは簡単だから処理できそう
# 最初にmappings, labelsをすべて集めて、次にラベルを置換する。2回入力アセンブリを読み込む

def gather(f):
    mappings = []  # list of (行番号, 命令番号)
    labels = []    # list of (ラベル, 行番号)
    l = 1  # line number
    i = 0  # inst numbe
    for line in f:
        if line[0] == '#':
            continue
        elif line.endswith(":\n"):
            labels.append((line[:-2], l))
        else:
            mappings.append((l, i))
            i += 1
        l += 1
    return mappings, labels

def subst_labels(mappings, labels, f):
    # returns the substituted list of instructions
    # 対象はj/jal/beq/bne

    def g(label):
        for (l,ll) in labels:
            if label in l:
                for (lll, i) in mappings:
                    if ll == lll-1:
                        return i
        return -1
    def nlin_to_ninst(i):
        for (nlin, inst) in mappings:
            if i == nlin:
                return inst
    ret = []
    l = 0
    for line in f:
        l += 1
        if line.strip()[-1:] == ':':
            continue
        #print(line)
        if line.strip().startswith('jal '):
            n = g(line.strip()[4:])
            line = 'jal ' + str(n) + '\n'
        elif line.strip().startswith('j '):
            n = g(line.strip()[2:])
            line = 'j ' + str(n) + '\n'
        elif line.strip().startswith('bne ') or line.strip().startswith('beq '):
            v = line.strip().split(' ')
            n = g(v[-1]) - nlin_to_ninst(l)
            line = v[0] + ' ' + v[1] + ' ' + v[2] + ' ' + str(n) + '\n'
        ret.append(line)
    return ret

path = 'foo.s'
out = 'piyo.s'

import sys

if __name__ == '__main__':
    if len(sys.argv) == 3:
        path = sys.argv[1]
        out = sys.argv[2]
    with open(path) as f:
        mappings, labels = gather(f)
    with open(path) as f:
        ret = subst_labels(mappings, labels, f)
        with open(out, "w") as writer:
            writer.writelines(ret)
