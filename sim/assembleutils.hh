#pragma once

#define $(i) (std::stoi(v[i]))
#define $r(i) (std::stoi(v[i].substr(1)))
#define $f(i) (std::stoi(v[i].substr(1)))

uint32_t encode_r(int opcode, int func, int rd, int ra, int rb, int shift);
uint32_t encode_i(int opcode, int rd, int ra, int imm);
uint32_t encode_j(int opcode, int addr);
