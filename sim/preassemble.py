#!/usr/bin/env python
# coding: utf-8

# 各命令の行番号と命令番号の対応をとるとともに、ラベルを定数に置換する
# (行番号, 命令番号) というpairを作れば、ラベルは行番号に置き換えは簡単だから処理できそう
# 最初にmappings, labelsをすべて集めて、次にラベルを置換する。2回入力アセンブリを読み込む

def gather(f):
    mappings = []  # list of (行番号, 命令番号)
    labels = []    # list of (ラベル, 行番号)
    l = 0  # line number
    i = 0  # inst number
    for line in f:
        l += 1
        if line.strip()[0] == '#':
            continue
        elif line.endswith(":\n"):
            labels.append((line[:-2], l))
        else:
            mappings.append((l, i))
            i += 1
    return mappings, labels

def label_to_nlin(labels, label):
    for (l, line) in labels:
        if label in l:
            return line
    return -1

def nlin_to_ninst(mappings, i):
    for (nlin, inst) in mappings:
        if i == nlin:
            return inst
    return -1

def label_to_ninst(labels, mappings, label):
    return nlin_to_ninst(mappings, 1 + label_to_nlin(labels, label))

def subst_labels(mappings, labels, f):
    # returns the substituted list of instructions
    ret = []
    l = 0
    for line in f:
        l += 1
        if line.strip()[-1:] == ':':
            continue
        #print(line)
        if line.strip().startswith('jal '):
            n = label_to_ninst(labels, mappings, line.strip()[4:])
            line = 'jal ' + str(n) + '\n'
        elif line.strip().startswith('bc1'): # bc1t/bc1f
            v = line.strip().split(' ')
            n = label_to_ninst(labels, mappings, v[-1]) - nlin_to_ninst(mappings, l) - 1
            line = v[0] + ' ' + str(n) + '\n'
        elif line.strip().startswith('j '):
            n = label_to_ninst(labels, mappings, line.strip()[2:])
            line = 'j ' + str(n) + '\n'
        elif line.strip().startswith('bne ') or line.strip().startswith('beq '):
            v = line.strip().split(' ')
            n = label_to_ninst(labels, mappings, v[-1]) - nlin_to_ninst(mappings, l) - 1
            line = v[0] + ' ' + v[1] + ' ' + v[2] + ' ' + str(n) + '\n'
        elif line.strip()[0] == '#':
            line = line.strip() + '\n'
        elif line.strip().startswith('mv'):
            v = line.strip().split(' ')
            line = 'or ' + v[1] + ' r0 ' + v[2] + '\n'
        elif line.strip().startswith('subi'):
            v = line.strip().split(' ')
            line = 'addi ' + v[1] + ' ' + v[2] + ' -' + v[3] + '\n'
        ret.append(line)
    return ret

def write_labels(path, labels, mappings):
    with open(path, "w") as f:
        buf = []
        for label,_ in labels:
            buf.append(label + ' ' + str(label_to_ninst(labels, mappings, label)) + '\n')
        f.writelines(buf)
    return buf

def write_ninsts(path, mappings):
    with open(path, "w") as f:
        buf = [str(ninst) + ' ' +  str(nlin) + '\n' for (nlin,ninst) in mappings]
        f.writelines(buf)

def halo16(lines, mappings, labels):
    ret = []
    for line in lines:
        while True:
            if 'ha16' not in line and 'lo16' not in line:
                break
            s = line.find('ha16(')
            if s != -1:
                e = line.find(')', s + len('ha16('))
                l = line[s + len('ha16('):e]
                ninst = label_to_ninst(labels, mappings, l)
                num = ninst & 0xf0
                line = line[:s] + str(num) + line[e+1:]
            s = line.find('lo16(')
            if s != -1:
                e = line.find(')', s + len('lo16('))
                l = line[s + len('lo16('):e]
                ninst = label_to_ninst(labels, mappings, l)
                num = ninst & 0x0f
                line = line[:s] + str(num) + line[e+1:]
        ret.append(line)
    return ret

path = 'foo.s'
out = 'piyo.s'

import sys

if __name__ == '__main__':
    #print(len(sys.argv))
    """
    if len(sys.argv) == 3:
        path = sys.argv[1]
        out = sys.argv[2]
    """
    labels = []
    mappings = []
    with open('label.txt') as f:
        for line in f:
            tmp = line[:-1].split(" ") # excluding '\n'
            labels += [(tmp[0], int(tmp[1]))]
    with open('inst.txt') as f:
        for line in f:
            tmp = line[:-1].split(" ") # excluding '\n'
            mappings += [(int(tmp[0]), int(tmp[1]))]
    #with open(path) as f:
    #    mappings, labels = gather(f)
    print(mappings)
    print(labels)
    with open(path) as f:
        ret = subst_labels(mappings, labels, f)
        ret = halo16(ret, mappings, labels)
        with open(out, "w") as writer:
            writer.writelines(ret)
    #write_labels('label.txt', labels, mappings)
    #write_ninsts('inst.txt', mappings)
