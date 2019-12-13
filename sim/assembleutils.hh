#pragma once

#define $(i) (std::stoi(v[i]))
#define $r(i) (std::stoi(v[i].substr(1)))
#define $f(i) (std::stoi(v[i].substr(1)))

void print_help(char *program_name);

uint32_t encode_r(int opcode, int func, int rd, int ra, int rb, int shift);
uint32_t encode_i(int opcode, int rd, int ra, int imm);
uint32_t encode_j(int opcode, int addr);

/** 1命令をstringで受け取り、その機械語を吐く */
uint32_t assemble(std::vector<std::string> v);
