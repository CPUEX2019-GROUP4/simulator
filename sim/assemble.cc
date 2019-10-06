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

#define $(i) (std::stoi(v[i]))
#define $r(i) (std::stoi(v[i].substr(1)))

// Prototypes
uint32_t encode_r(int opcode, int rs, int rt, int rd, int shift, int func);
uint32_t encode_i(int opcode, int rs, int rt, int imm);
uint32_t encode_r(int opcode, int rs, int rt, int rd, int shift, int func);
uint32_t assemble(std::string inst);

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
uint32_t assemble(std::vector<std::string> v)
{
  uint32_t ret;


  std::string op = v[0];

  // Arithmetic
  if (!op.compare("add")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x20);
  else if (!op.compare("sub")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x22);
  else if (!op.compare("addi")) ret = encode_i(0x08, $r(2), $r(1), $(3));
  else if (!op.compare("subi")) ret = encode_i(0x18, $r(2), $r(1), $(3)); // subi
  else if (!op.compare("mult")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x18);
  else if (!op.compare("div")) ret = encode_r(0x00, $r(1), $r(2), 0x00, 0x00, 0x1a);
  // Load/Store
  else if (!op.compare("lw")) ret = encode_i(0x23, $r(3), $r(1), $r(2));
  else if (!op.compare("lh")) ret = encode_i(0x21, $r(3), $r(1), $r(2));
  else if (!op.compare("lb")) ret = encode_i(0x20, $r(3), $r(1), $r(2));
  else if (!op.compare("sw")) ret = encode_i(0x2b, $r(3), $r(1), $r(2));
  else if (!op.compare("sh")) ret = encode_i(0x29, $r(3), $r(1), $r(2));
  else if (!op.compare("sb")) ret = encode_i(0x28, $r(3), $r(1), $r(2));
  else if (!op.compare("lui")) ret = encode_i(0x0f, 0x00, $r(1), $r(2));
  else if (!op.compare("mfhi")) ret = encode_r(0x00, 0x00, 0x00, $r(1), 0x00, 0x10);
  else if (!op.compare("mflo")) ret = encode_r(0x00, 0x00, 0x00, $r(1), 0x00, 0x12);
  // Logic
  else if (!op.compare("and")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x24);
  else if (!op.compare("andi")) ret = encode_i(0x0c, $r(2), $r(1), $(3));
  else if (!op.compare("or")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x25);
  else if (!op.compare("ori")) ret = encode_i(0x0d, $r(2), $r(1), $(3));
  else if (!op.compare("xor")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x26);
  else if (!op.compare("nor")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x27);
  else if (!op.compare("slt")) ret = encode_r(0x00, $r(2), $r(3), $r(1), 0x00, 0x2a);
  else if (!op.compare("slti")) ret = encode_i(0x0a, $r(2), $r(1), $(3));
  // Shift
  else if (!op.compare("sll")) ret = encode_r(0x00, 0x00, $r(2), $r(1), $(3), 0x00);
  else if (!op.compare("srl")) ret = encode_r(0x00, 0x00, $r(2), $r(1), $(3), 0x02);
  else if (!op.compare("sra")) ret = encode_r(0x00, 0x00, $r(2), $r(1), $(3), 0x03);
  else if (!op.compare("sllv")) ret = encode_r(0x00, $r(3), $r(2), $r(1), 0x00, 0x04);
  // Jump
  else if (!op.compare("beq")) ret = encode_i(0x04, $r(1), $r(2), $(3));
  else if (!op.compare("bne")) ret = encode_i(0x05, $r(1), $r(2), $(3));
  else if (!op.compare("j")) ret = encode_j(0x02, $(1));
  else if (!op.compare("jr")) ret = encode_r(0x00, $r(1), 0x00, 0x00, 0x00, 0x08);
  else if (!op.compare("jal")) ret = encode_j(0x03, $(1));
  // Others
  else if (!op.compare("nop")) ret = encode_r(0x09, 0x00, 0x00, 0x00, 0x00, 0x00); // nop

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

  std::string inst;

  while (!ifs.eof()) {
    getline(ifs, inst);
    if (!inst.compare("")) break;

    // split input string by ' ' and save them to a vector v
    std::vector<std::string> v;
    std::stringstream ss{inst};
    std::string buf;
    while (std::getline(ss, buf, ' ')) {
      if (!buf.compare("")) continue;
      v.push_back(buf);
    }
    if (!v[0].compare(0, 1, "#")) continue;   // lines which start with '#' are comments

    std::cout << inst << " --> ";

    uint32_t ret = assemble(v);

    std::cout << ret << std::endl;

    ofs.write((char*)(&ret), 4);
  }

  ifs.close();
  ofs.close();

  printf("Assembled successfully.\n");

  return 0;
}
