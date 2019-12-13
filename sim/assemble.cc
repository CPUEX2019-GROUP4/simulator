#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>

#include "stringutils.hh"
#include "assembleutils.hh"

//#define $(i) (std::stoi(v[i]))
//#define $r(i) (std::stoi(v[i].substr(1)))
//#define $f(i) (std::stoi(v[i].substr(1)))


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
  // Logic
  else if (!op.compare("or")) ret = encode_r(0x00, 0x25, $r(1), $r(2), $r(3), 0x00);
  else if (!op.compare("ori")) ret = encode_i(0x0d, $r(1), $r(2), $(3));
  else if (!op.compare("slt")) ret = encode_r(0x00, 0x2a, $r(1), $r(2), $r(3), 0x00);
  else if (!op.compare("slti")) ret = encode_i(0x0a, $r(1), $r(2), $(3));
  // Shift
  else if (!op.compare("sll")) ret = encode_r(0x00, 0x00, $r(1), $r(2), 0x00, $(3));
  else if (!op.compare("sllv")) ret = encode_r(0x00, 0x04, $r(1), $r(2), $r(3), 0x00);
  // Jump
  else if (!op.compare("beq")) ret = encode_i(0x04, $r(1), $r(2), $(3));
  else if (!op.compare("bne")) ret = encode_i(0x05, $r(1), $r(2), $(3));
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
  else if (!op.compare("fclt")) ret = encode_r(0x11, 0x20, 0x00, $f(1), $f(2), 0x00);
  else if (!op.compare("fcz")) ret = encode_r(0x11, 0x28, 0x00, $f(1), 0x00, 0x00);
  else if (!op.compare("fmv")) ret = encode_r(0x11, 0x06, $f(1), $f(2), 0x00, 0x00);
  else if (!op.compare("sqrt_init")) ret = encode_r(0x11, 0x30, $f(1), $f(2), 0x00, 0x00);
  else if (!op.compare("finv_init")) ret = encode_r(0x11, 0x38, $f(1), $f(2), 0x00, 0x00);
  else if (!op.compare("lwcZ")) ret = encode_i(0x30, $f(1), $r(2), $(3));
  else if (!op.compare("swcZ")) ret = encode_i(0x38, $f(1), $r(2), $(3));
  else if (!op.compare("bc1t")) ret = encode_i(0x13, 0x08, 0x01, $(1)); // XXX
  else if (!op.compare("bc1f")) ret = encode_i(0x15, 0x08, 0x00, $(1));
  else if (!op.compare("ftoi")) ret = encode_i(0x1c, $r(1), $f(2), 0x00);
  else if (!op.compare("itof")) ret = encode_i(0x1d, $f(1), $r(2), 0x00);
  else if (!op.compare("flui")) ret = encode_i(0x3c, $f(1), 0x00, $(2));
  else if (!op.compare("fori")) ret = encode_i(0x3d, $f(1), $f(2), $(3));
  // Others
  else if (!op.compare("nop")) ret = encode_r(0x00, 0x00, 0x00, 0x00, 0x00, 0x00); // nop
  else if (!op.compare("out")) ret = encode_i(0x3f, $r(1), 0x00, $(2));
  else if (!op.compare("inint")) ret = encode_r(0x18, 0x00, $r(1), 0x00, 0x00, 0x00);
  else if (!op.compare("inflt")) ret = encode_r(0x19, 0x00, $r(1), 0x00, 0x00, 0x00);

  else {std::cerr << "\033[1m Unknown instruction. Abort.\033[m" << /*inst <<*/ std::endl; exit(1);}

  return ret;
}


void print_help(char *program_name)
{
  printf("USAGE: %s {{source}} {{destination}}\n"
         "If destination is not given, a.out is used instead.\n", program_name);
}

int main(int argc, char **argv)
{
  if (argc != 3 && argc != 2) {
    print_help(argv[0]);
    exit(1);
  }

  std::ifstream ifs(argv[1]);
  std::ofstream ofs;

  if (argc >= 3) ofs.open(argv[2], std::ios::out|std::ios::binary|std::ios::trunc);
  else {
    printf("output file is to be saved to 'a.out'\n");
    ofs.open("a.out", std::ios::out|std::ios::binary|std::ios::trunc);
  }

  if (ifs.fail()) {std::cerr << "failed to open " << argv[1] << "\n"; exit(1);}
  if (ofs.fail()) {std::cerr << "failed to open " << argv[2] << "\n"; exit(1);}

  std::cout << "------------------------------\n";

  std::string inst;

  while (!ifs.eof()) {
    getline(ifs, inst);
    if (!inst.compare("")) break;

    std::vector<std::string> v;
    inst = split(inst, "#", false)[0];
    v = split(inst, " ");
    if (v.empty()) continue;

    try {
      uint32_t ret = assemble(v);
      ofs.write((char*)(&ret), 4);
    }
    catch (const int& e) {
      std::cerr << "imm was " << e << " at the instruction '" << inst << "'\n";
      exit(1);
    }
  }

  ifs.close();
  ofs.close();

  printf("Assembled successfully.\n");
  printf("%s ==> %s\n", argv[1], argv[2]);

  return 0;
}
