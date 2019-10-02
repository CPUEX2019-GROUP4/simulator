/** Assember for MIPS.
 * currently assuming that aseembly language is not separated by ',',
 * but by mere whitespaces. E.g. addi r1 r0 300 (not: addi r1,r0,300)
 **/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>

#define COMPILE_R(opcode,shift,func) (encode_r(opcode, std::stoi(v[2].substr(1)), std::stoi(v[3].substr(1)), std::stoi(v[1].substr(1)), shift, func))
#define COMPILE_I(opcode) (encode_i(opcode, std::stoi(v[3].substr(1)), std::stoi(v[1].substr(1)), std::stoi(v[2])))
#define COMPILE_J(opcode) (encode_j(opcode, std::stoi(v[1])))

// Prototypes
uint32_t encode_r(int opcode, int rs, int rt, int rd, int shift, int func);
uint32_t encode_i(int opcode, int rs, int rt, int imm);
uint32_t encode_r(int opcode, int rs, int rt, int rd, int shift, int func);
uint32_t assemble(std::string inst);
void print_assemble(std::string inst, uint32_t enc);


void print_assemble(std::string inst, uint32_t enc)
{
  std::cout << inst << " --> " << enc << std::endl;
}

uint32_t encode_r(int opcode, int rs, int rt, int rd, int shift, int func)
{
  //printf("opcode: %d, rd: %d, rs: %d, rt: %d, shift: %d, func: %d\n", opcode,
  //       rd, rs, rt, shift, func);
  uint32_t ret = 0;

  if (opcode >= 64) {std::cerr << "opcode (" << opcode  << ") >= 64\n";exit(1);}
  else ret |= (opcode & 0x3f) << 26;
  if (rs >= 32) {std::cerr << "rs (" << rs << ") >= 32\n";exit(1);}
  else ret |= (rs & 0x1f) << 21;
  if (rt >= 32) {std::cerr << "rt (" << rt << ") >= 32\n";exit(1);}
  else ret |= (rt & 0x1f) << 16;
  if (rd >= 32) {std::cerr << "rd (" << rd << ") >= 32\n";exit(1);}
  else ret |= (rd & 0x1f) << 11;
  if (shift >= 32) {std::cerr << "shift (" << shift << ") >= 32\n";exit(1);}
  else ret |= (shift & 0x1f) << 6;
  if (func >= 64) {std::cerr << "func (" << func << ") >= 64\n";exit(1);}
  else ret |= (func & 0x3f);

  return ret;
}

uint32_t encode_i(int opcode, int rs, int rt, int imm)
{
  //printf("opcode: %d, rt: %d, imm: %d, rs: %d\n", opcode, rt, imm, rs);
  uint32_t ret = 0;

  if (opcode >= 64) {std::cerr << "opcode (" << opcode  << ") >= 64\n";exit(1);}
  else ret |= (opcode & 0x3f) << 26;
  if (rs >= 32) {std::cerr << "rs (" << rs << ") >= 32\n";exit(1);}
  else ret |= (rs & 0x1f) << 21;
  if (rt >= 32) {std::cerr << "rt (" << rt << ") >= 32\n";exit(1);}
  else ret |= (rt & 0x1f) << 16;
  if (imm >= 65536) {std::cerr << "immediate (" << imm << ") >= 65536\n";exit(1);}
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
uint32_t assemble(std::string inst)
{
  uint32_t ret;

  // split input string by ' ' and save them to a vector v
  std::vector<std::string> v;
  std::stringstream ss{inst};
  std::string buf;
  while (std::getline(ss, buf, ' ')) v.push_back(buf); // XXX: really whitespace as separator?

  if (v.empty()) {printf("empty"); exit(1); /*return -1;*/} // empty. (unexpected)

  std::string op = v[0];

  // Arithmetic
  if (!op.compare("add")) ret = COMPILE_R(0x00, 0x00, 0x20); // add
  else if (!op.compare("sub")) ret = COMPILE_R(0x00, 0x00, 0x22); // sub
  else if (!op.compare("addi")) ret = COMPILE_R(0x08, 0x00, 0x00); // addi
  else if (!op.compare("mult")) ret = COMPILE_R(0x00, 0x00, 0x18); // mult
  else if (!op.compare("div")) ret = COMPILE_R(0x00, 0x00, 0x1a); // div
  // Load/Store
  else if (!op.compare("lw")) ret = COMPILE_I(0x23);  // lw
  else if (!op.compare("lh")) ret = COMPILE_I(0x21);  // lh
  else if (!op.compare("lb")) ret = COMPILE_I(0x20);  // lb
  else if (!op.compare("sw")) ret = COMPILE_I(0x2b);  // sw
  else if (!op.compare("sh")) ret = COMPILE_I(0x29);  // sh
  else if (!op.compare("sb")) ret = COMPILE_I(0x28);  // sb
  else if (!op.compare("lui")) ret = COMPILE_I(0x0f);  // lui
  // Logic
  else if (!op.compare("and")) ret = COMPILE_R(0x00, 0x00, 0x24); // and
  else if (!op.compare("andi")) ret = COMPILE_I(0x0c);  // andi
  else if (!op.compare("or")) ret = COMPILE_R(0x00, 0x00, 0x25); // or
  else if (!op.compare("ori")) ret = COMPILE_I(0x0d);  // ori
  else if (!op.compare("xor")) ret = COMPILE_R(0x00, 0x00, 0x26); // xor
  else if (!op.compare("nor")) ret = COMPILE_R(0x00, 0x00, 0x27); // nor
  else if (!op.compare("slt")) ret = COMPILE_R(0x00, 0x00, 0x2a); // slt
  else if (!op.compare("slti")) ret = COMPILE_I(0x0a);  // slti
  // Shift
  else if (!op.compare("sll")) ret = COMPILE_R(0x00, 0x00, 0x00); // sll
  else if (!op.compare("srl")) ret = COMPILE_R(0x00, 0x00, 0x02); // srl
  else if (!op.compare("sra")) ret = COMPILE_R(0x00, 0x00, 0x03); // sra
  // Jump
  else if (!op.compare("beq")) ret = COMPILE_I(0x04);  // beq
  else if (!op.compare("bne")) ret = COMPILE_I(0x05);  // bne
  else if (!op.compare("j")) ret = COMPILE_J(0x02);  // j
  else if (!op.compare("jr")) ret = COMPILE_R(0x00, 0x00, 0x08); // jr
  else if (!op.compare("jal")) ret = COMPILE_J(0x03);  // jal
  // Others
  else {std::cerr << "Unknown instruction: " << inst << std::endl; exit(1);}

  print_assemble(inst, ret);

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

  std::string inst;

  while (!ifs.eof()) {
    getline(ifs, inst);
    if (!inst.compare("")) break;
    uint32_t ret = assemble(inst);
    ofs.write((char*)(&ret), 4);
  }

  ifs.close();
  ofs.close();

  printf("Assembled successfully.\n");

  return 0;
}
