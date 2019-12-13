#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>
#include "assembleutils.hh"

//#define $(i) (std::stoi(v[i]))
//#define $r(i) (std::stoi(v[i].substr(1)))
//#define $f(i) (std::stoi(v[i].substr(1)))

uint32_t encode_r(int opcode, int func, int rd, int ra, int rb, int shift)
{
  //printf("opcode: %d, rd: %d, ra: %d, rb: %d, shift: %d, func: %d\n", opcode,
  //       rd, ra, rb, shift, func);
  uint32_t ret = 0;

  if (opcode >= 64) {std::cerr << "opcode (" << opcode << ") >= 64\n";exit(1);}
  else ret |= (opcode & 0x3f) << 26;
  if (rd >= 32) {std::cerr << "rd (" << rd << ") >= 32\n";exit(1);}
  else ret |= (rd & 0x1f) << 21;
  if (ra >= 32) {std::cerr << "ra (" << ra << ") >= 32\n";exit(1);}
  else ret |= (ra & 0x1f) << 16;
  if (rb >= 32) {std::cerr << "rb (" << rb << ") >= 32\n";exit(1);}
  else ret |= (rb & 0x1f) << 11;
  if (shift >= 32) {std::cerr << "shift (" << shift << ") >= 32\n";exit(1);}
  else ret |= (shift & 0x1f) << 6;
  if (func >= 64) {std::cerr << "func (" << func << ") >= 64\n";exit(1);}
  else ret |= func & 0x3f;

  return ret;
}

uint32_t encode_i(int opcode, int rd, int ra, int imm)
{
  //printf("opcode: %d, rb: %d, imm: %d, ra: %d\n", opcode, rb, imm, ra);
  uint32_t ret = 0;

  if (opcode >= 64) {std::cerr << "opcode (" << opcode  << ") >= 64\n";exit(1);}
  else ret |= (opcode & 0x3f) << 26;
  if (rd >= 32) {std::cerr << "rd (" << rd << ") >= 32\n";exit(1);}
  else ret |= (rd & 0x1f) << 21;
  if (ra >= 32) {std::cerr << "ra (" << ra << ") >= 32\n";exit(1);}
  else ret |= (ra & 0x1f) << 16;
  if (imm >= 65536) {std::cerr << "immediate (" << imm << ") >= 65536\n"; throw imm;/*exit(1);*/}
  else ret |= (imm & 0xffff);

  return ret;
}

uint32_t encode_j(int opcode, int addr)
{
  //printf("opcode: %d, addr: %d\n", opcode, addr);
  uint32_t ret = 0;

  if (opcode >= 64) {std::cerr << "opcode (" << opcode  << ") >= 64\n";exit(1);}
  else ret |= (opcode & 0x3f) << 26;
  if (addr>= 67108864) {std::cerr << "addr (" << addr << ") >= 67108864\n";exit(1);}
  else ret |= (addr & 0x3ffffff);

  return ret;
}
