#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>

#include "assembleutils.hh"

void print_help(char *program_name)
{
  printf("USAGE: %s {{source}} {{destination}}\n"
         "If destination is not given, a.out is used instead.\n", program_name);
}

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

/** 1命令をstringで受け取り、その機械語を吐く */
uint32_t assemble(std::vector<std::string> v)
{
  uint32_t ret;

  std::string op = v[0];

  // Arithmetic
  if (!op.compare("add")) ret = encode_r(0x00, 0x20, $r(1), $r(2), $r(3), 0x00);
  else if (!op.compare("sub")) ret = encode_r(0x00, 0x22, $r(1), $r(2), $r(3), 0x00);
  else if (!op.compare("addi")) ret = encode_i(0x08, $r(1), $r(2), $(3));
  else if (!op.compare("div2")) ret = encode_r(0x00, 0x0c, $r(1), $r(2), 0x00, 0x00);
  else if (!op.compare("div10")) ret = encode_r(0x00, 0x1c, $r(1), $r(2), 0x00, 0x00);
  // Load/Store
  else if (!op.compare("lw")) ret = encode_i(0x23, $r(1), $r(2), $(3));
  else if (!op.compare("sw")) ret = encode_i(0x2b, $r(1), $r(2), $(3));
  else if (!op.compare("lui")) ret = encode_i(0x0f, $r(1), 0x00, $(2));
  else if (!op.compare("lwab")) ret = encode_r(0x00, 0x33, $r(1), $r(2), $r(3), 0x00);  // 2nd
  else if (!op.compare("swab")) ret = encode_r(0x00, 0x3b, $r(1), $r(2), $r(3), 0x00);  // 2nd
  // Logic
  else if (!op.compare("or")) ret = encode_r(0x00, 0x25, $r(1), $r(2), $r(3), 0x00);
  else if (!op.compare("ori")) ret = encode_i(0x0d, $r(1), $r(2), $(3));
  else if (!op.compare("slt")) ret = encode_r(0x00, 0x2a, $r(1), $r(2), $r(3), 0x00);
  else if (!op.compare("slti")) ret = encode_i(0x0a, $r(1), $r(2), $(3));
  else if (!op.compare("seq")) ret = encode_r(0x00, 0x2b, $r(1), $r(2), $r(3), 0x00);   // 2nd
  // Shift
  else if (!op.compare("sll")) ret = encode_r(0x00, 0x00, $r(1), $r(2), 0x00, $(3));
  else if (!op.compare("sllv")) ret = encode_r(0x00, 0x04, $r(1), $r(2), $r(3), 0x00);
  // Jump
  else if (!op.compare("beq")) ret = encode_i(0x04, $r(1), $r(2), $(3));
  else if (!op.compare("bne")) ret = encode_i(0x05, $r(1), $r(2), $(3));
  else if (!op.compare("blt")) ret = encode_i(0x06, $r(1), $r(2), $(3));    // 2nd
  else if (!op.compare("j")) ret = encode_j(0x02, $(1));
  else if (!op.compare("jr")) ret = encode_r(0x00, 0x08, $r(1), 0x00, 0x00, 0x00);
  else if (!op.compare("jal")) ret = encode_j(0x03, $(1));
  else if (!op.compare("jalr")) ret = encode_r(0x00, 0x0f, $r(1), 0x00, 0x00, 0x00);
  // Floating point
  else if (!op.compare("fneg")) ret = encode_r(0x11, 0x10, $f(1), $f(2), 0x00, 0x00);
  else if (!op.compare("fadd")) ret = encode_r(0x11, 0x00, $f(1), $f(2), $f(3), 0x00);
  else if (!op.compare("fsub")) ret = encode_r(0x11, 0x01, $f(1), $f(2), $f(3), 0x00);
  else if (!op.compare("fmul")) ret = encode_r(0x11, 0x02, $f(1), $f(2), $f(3), 0x00);
  else if (!op.compare("fdiv")) ret = encode_r(0x11, 0x03, $f(1), $f(2), $f(3), 0x00);
  else if (!op.compare("feq")) ret = encode_r(0x11, 0x29, 0x00, $f(1), $f(2), 0x00);  // 2nd
  else if (!op.compare("fclt")) ret = encode_r(0x11, 0x20, 0x00, $f(1), $f(2), 0x00);
  else if (!op.compare("fcz")) ret = encode_r(0x11, 0x28, 0x00, $f(1), 0x00, 0x00);
  else if (!op.compare("fmv")) ret = encode_r(0x11, 0x06, $f(1), $f(2), 0x00, 0x00);
  else if (!op.compare("sqrt_init")) ret = encode_r(0x11, 0x30, $f(1), $f(2), 0x00, 0x00);
  else if (!op.compare("finv_init")) ret = encode_r(0x11, 0x38, $f(1), $f(2), 0x00, 0x00);
  else if (!op.compare("lwcZ")) ret = encode_i(0x30, $f(1), $r(2), $(3));
  else if (!op.compare("swcZ")) ret = encode_i(0x38, $f(1), $r(2), $(3));
  else if (!op.compare("bc1t")) ret = encode_i(0x13, 0x08, 0x01, $(1)); // XXX
  else if (!op.compare("bc1f")) ret = encode_i(0x15, 0x08, 0x00, $(1));
  else if (!op.compare("fabs")) ret = encode_i(0x1e, $r(1), $f(2), 0x00); // 2nd
  else if (!op.compare("ftoi")) ret = encode_i(0x1c, $r(1), $f(2), 0x00);
  else if (!op.compare("itof")) ret = encode_i(0x1d, $f(1), $r(2), 0x00);
  else if (!op.compare("flui")) ret = encode_i(0x3c, $f(1), 0x00, $(2));
  else if (!op.compare("fori")) ret = encode_i(0x3d, $f(1), $f(2), $(3));
  else if (!op.compare("flwab")) ret = encode_r(0x11, 0x33, $f(1), $r(2), $r(3), 0x00);  // 2nd
  else if (!op.compare("fswab")) ret = encode_r(0x11, 0x3b, $f(1), $r(2), $r(3), 0x00);  // 2nd
  else if (!op.compare("flw")) ret = encode_i(0x30, $f(1), $r(2), $(3));  // 2nd
  else if (!op.compare("fsw")) ret = encode_i(0x38, $f(1), $r(2), $(3));  // 2nd
  // Others
  else if (!op.compare("nop")) ret = encode_r(0x00, 0x00, 0x00, 0x00, 0x00, 0x00); // nop
  else if (!op.compare("out")) ret = encode_i(0x3f, $r(1), 0x00, $(2));
  else if (!op.compare("inint")) ret = encode_r(0x18, 0x00, $r(1), 0x00, 0x00, 0x00);
  else if (!op.compare("inflt")) ret = encode_r(0x19, 0x00, $r(1), 0x00, 0x00, 0x00);

  else {std::cerr << "\033[1m Unknown instruction: "; throw op;}

  return ret;
}
