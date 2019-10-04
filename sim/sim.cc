/** 機械語ファイルをコマンドライン入力で受け取り、ステップ実行結果を表示する。
 *  Big Endianの機械語を想定(たぶん).
 **/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

#define BYTES_INSTRUCTION 32
#define LEN_INSTRUCTION 10000
#define N_REG 32
#define SIZE_MEM 4096

#define DELAY_SLOT 4

#define $s (int_reg[get_rs(inst)])
#define $t (int_reg[get_rt(inst)])
#define $d (int_reg[get_rd(inst)])

enum Comm {STEP, PRINT, BREAK, HELP, NIL, ERR, NOP, OP};

// Parameters
const char PROMPT[] = "(ryo) ";

// Prototypes
void show_help(void);
void print_regs(void);
void init_inst(char *pathname);
enum Comm exec_inst(uint32_t);
enum Comm exec_inst(void);
enum Comm read_commands(void);

// registers
uint32_t *inst_reg;           // instruction register
int pc = 0;                   // program counter
int32_t int_reg[N_REG];       // int
float   float_reg[N_REG];     // float
int32_t LO, HI;               // special registers
std::array<char, SIZE_MEM> mem;           // memory

// Meta variables
//std::unordered_set<int> regs_to_show;    // 表示させるレジスタたち
//std::unordered_set<int> fregs_to_show;   // (浮動小数)表示させるレジスタたち
std::vector<int> regs_to_show;    // 表示させるレジスタたち
std::vector<int> fregs_to_show;   // (浮動小数)表示させるレジスタたち
int total_inst = 0;

/** show help */
void show_help(void)
{
  puts("available commands.");
  puts("-------------------");
  printf("step (s): \t\texecute the current instruction of the binary.\n");
  printf("print (p) {{rn}}: \tadd the register to reserved list to show.\n"
         "                  \tE.g. `p r0`, `p f2`, `p LO`.\n");
  printf("break (b): \t\tnot implemented yet.\n");
	printf("exit/quit \t\tterminate the simulator\n");
  puts("-------------------");
}

/**--- print registers ---*/
void print_regs(void)
{
  int count = 0;
  for (auto it : regs_to_show) {
    if (it == -1) printf("\t%d: LO = %d\n", ++count, LO); // XXX: LO/HI は-1,-2で表現(zako-coder)
    else if (it == -2) printf("\t%d: HI = %d\n", ++count, HI);
    else printf("\t%d: r%d = %d\n", ++count, it, int_reg[it]);
  }
  //putchar('\n');
  for (auto it : fregs_to_show) {
    printf("\t%d f%d = %f\n",  ++count, it, float_reg[it]);
  }
}

/**--- strcpy()のnull文字付加なしver ---*/
int copy(char *dst, char *src, int n)
{
  int i = 0;
  for (; i<n; i++) dst[i] = src[i];
  return i;
}

/**--- read instructions from a file and save them to inst_reg ---*/
void init_inst(char *pathname)
{
  FILE *fin;

  fin = fopen(pathname, "rb");
  if (fin == NULL) {perror("fopen"); exit(1);}

  inst_reg = (uint32_t*) malloc(LEN_INSTRUCTION * 4);  // 4 bytes per instruction
  if (inst_reg == NULL) {perror("malloc"); exit(1);}
  total_inst = fread(inst_reg, 1, LEN_INSTRUCTION, fin) / 4;
  printf("loaded %d instructions.\n", total_inst);

  if (fclose(fin)) {perror("fclose"); exit(1);}

  return;
}

uint32_t get_opcode(uint32_t inst) {return (inst >> 26) & 0x3f;}
uint32_t get_rs(uint32_t inst) {return (inst >> 21) & 0x1f;}
uint32_t get_rt(uint32_t inst) {return (inst >> 16) & 0x1f;}
uint32_t get_rd(uint32_t inst) {return (inst >> 11) & 0x1f;}
uint32_t get_shift(uint32_t inst) {return (inst >> 6) & 0x1f;}
uint32_t get_funct(uint32_t inst) {return (inst >> 0) & 0x3f;}
uint32_t get_imm(uint32_t inst) {return (inst >> 0) & 0x0fff;}
int32_t get_imm_signed(uint32_t inst) {return (inst >> 0) & 0x0fff;}     // signed
uint32_t get_addr(uint32_t inst) {return (inst >> 0) & 0x3ffffff;}

/** execute single instruction */
enum Comm exec_inst(void)
{
  if (!(0 <= pc && pc < total_inst)) {
    printf("pc out of index. abort. %d of %d\n", pc, total_inst);
    exit(1);
  }
  printf("%d: ", pc);
  return exec_inst(inst_reg[pc]);
}

/** execute single instruction */
enum Comm exec_inst(uint32_t inst)
{
  switch (get_opcode(inst)) {
    case 0x09:
      printf("nop\n");
      return NOP;   // nop
    case 0x00:
      switch (get_funct(inst)) {
        case 0x00:  // sll
          printf("sll r%d, r%d, %d\n", get_rd(inst), get_rt(inst), get_imm(inst));
          $d = $t << get_imm(inst);
          pc++; break;
        case 0x02:  // srl
          printf("srl r%d, r%d, %d\n", get_rd(inst), get_rt(inst), get_imm(inst));
          $d = $t >> get_imm(inst); //XXX check if 0 is extended
          pc++; break;
        case 0x03:  // sra
          printf("sra r%d, r%d, %d\n", get_rd(inst), get_rt(inst), get_imm(inst));
          $d = $t >> get_imm(inst); //XXX check if sign is extended
          pc++; break;
        case 0x04:  // sllv
          printf("sllv r%d, r%d, r%d\n", get_rd(inst), get_rt(inst), get_rs(inst));
          $d = $t << $s;
          pc++; break;
        case 0x1a:  // div
          printf("div r%d, r%d\n", get_rs(inst), get_rt(inst));
          LO = $s / $t;
          HI = $s % $t;
          pc++; break;
        case 0x1b:  // divu
          printf("divu r%d, r%d\n", get_rs(inst), get_rt(inst));
          LO = (unsigned)$s / (unsigned)$t;
          HI = (unsigned)$s % (unsigned)$t;
          pc++; break;
        case 0x08:  // jr
          printf("jr r%d\n", get_rs(inst));
          pc = $s;
          break;
        case 0x10:  // mfhi
          printf("mfhi r%d\n", get_rd(inst));
          $d = HI;
          pc++; break;
        case 0x12:  // mflo
          printf("mflo r%d\n", get_rd(inst));
          $d = LO;
          pc++; break;
        case 0x20:  // add
          printf("add r%d, r%d, r%d\n", get_rd(inst), get_rs(inst), get_rt(inst));
          $d = $s + $t;
          pc++; break;
        case 0x22:  // sub
          printf("sub r%d, r%d, r%d\n", get_rd(inst), get_rs(inst), get_rt(inst));
          $d = $s - $t;
          pc++; break;
        case 0x24:  // and
          printf("and r%d, r%d, r%d\n", get_rd(inst), get_rs(inst), get_rt(inst));
          $d = $s & $t;
          pc++; break;
        case 0x25:  // or
          printf("or r%d, r%d, r%d\n", get_rd(inst), get_rs(inst), get_rt(inst));
          $d = $s | $t;
          pc++; break;
        case 0x26:  // xor
          printf("xor r%d, r%d, r%d\n", get_rd(inst), get_rs(inst), get_rt(inst));
          $d = $s ^ $t;
          pc++; break;
        case 0x27:  // nor
          printf("nor r%d, r%d, r%d\n", get_rd(inst), get_rs(inst), get_rt(inst));
          $d = ~($s | $t);
          pc++; break;
        case 0x2a:  // slt
          printf("slt r%d, r%d, r%d\n", get_rd(inst), get_rs(inst), get_rt(inst));
          $d = ($s < $t)? 1: 0;
          pc++; break;
        default:
          fprintf(stderr, "Unknown funct: %d\n", get_funct(inst));
          exit(1);
      }
      break;
    case 0x0a:      // slti
      printf("slti r%d, r%d, %d\n", get_rt(inst), get_rs(inst), get_imm(inst));
      $t = ($s < get_imm_signed(inst))? 1: 0;  // XXX: 符号拡張されている?
      pc++; break;
    case 0x0c:      // andi
      printf("andi r%d, r%d, %d\n", get_rt(inst), get_rs(inst), get_imm(inst));
      $t = $s & get_imm(inst);
      pc++; break;
    case 0x0d:      // ori
      printf("ori r%d, r%d, %d\n", get_rt(inst), get_rs(inst), get_imm(inst));
      $t = $s | get_imm_signed(inst);    // XXX: Cが符号拡張されているかcheck
      pc++; break;
    case 0x0f:      // lui
      printf("lui r%d, %d\n", get_rt(inst), get_imm(inst));
      $t = get_imm(inst) << 16;
      pc++; break;
    case 0x02:      // j
      printf("j %d\n", get_addr(inst));
      pc = get_addr(inst);
      break;
    case 0x23:      // lw
      printf("lw r%d, %d(r%d)\n", get_rt(inst), get_imm(inst), get_rs(inst));
      copy((char*)(&($t)), &mem[$s + get_imm_signed(inst)], 4);
      pc++; break;
    case 0x21:      // lh
      printf("lh r%d, %d(r%d)\n", get_rt(inst), get_imm(inst), get_rs(inst));
      copy((char*)(&($t)), &mem[$s + get_imm_signed(inst)], 2);  // XXX: 符号拡張されているかcheck
      pc++; break;
    case 0x20:      // lb
      printf("lb r%d, %d(r%d)\n", get_rt(inst), get_imm(inst), get_rs(inst));
      copy((char*)(&($t)), &mem[$s + get_imm_signed(inst)], 1);  // XXX: 符号拡張されているかcheck
      pc++; break;
    case 0x2b:      // sw
      printf("sw r%d, %d(r%d)\n", get_rt(inst), get_imm(inst), get_rs(inst));
      copy(&mem[$s + get_imm_signed(inst)], (char*)(&($t)), 4);
      pc++; break;
    case 0x29:      // sh
      printf("sh r%d, %d(r%d)\n", get_rt(inst), get_imm(inst), get_rs(inst));
      copy(&mem[$s + get_imm(inst)], (char*)(&($t)), 2);  // XXX: 符号拡張されているかcheck
      pc++; break;
    case 0x28:      // sb
      printf("sb r%d, %d(r%d)\n", get_rt(inst), get_imm(inst), get_rs(inst));
      copy(&mem[$s + get_imm(inst)], (char*)(&($t)), 1);  // XXX: 符号拡張されているかcheck
      pc++; break;
    case 0x03:      // jal
      printf("jal %d\n", get_imm(inst));
      int_reg[31] = pc + 1;   // XXX: 遅延スロットを使うなら+2にする
      pc += ((int_reg[DELAY_SLOT] >> 28) & 0x1f) | get_addr(inst);
      break;
    case 0x04:      // beq
      printf("beq r%d, r%d, %d\n", get_rs(inst), get_rt(inst), get_imm(inst));
      if ($s == $t) pc += 1 + get_imm(inst);
      else pc++;
      break;
    case 0x05:      // bne
      printf("bne r%d, r%d, %d\n", get_rs(inst), get_rt(inst), get_imm(inst));
      if ($s != $t) pc += 1 + get_imm(inst);
      else pc++;
      break;
    case 0x08:      // addi
      printf("addi r%d, r%d, %d\n", get_rt(inst), get_rs(inst), get_imm(inst));
      $t = $s + get_imm(inst);
      pc++; break;
    default:
      fprintf(stderr, "Unknown opcode: 0x%x\n", get_opcode(inst));
      exit(1);
  }
  return OP;
}

/**--- read commands from stdin ---*/
enum Comm read_commands(void)
{
  printf("%s", PROMPT);

  std::vector<std::string> v;
  std::string s;

  std::getline(std::cin, s);  // input

  // split input string by ' ' and save them to a vector v
  std::stringstream ss{s};
  std::string buf;
  while (std::getline(ss, buf, ' ')) v.push_back(buf);

  if (!s.compare("") || !s.compare("exit") || !s.compare("quit")) return NIL;

  std::string comm = v[0];

  if (comm == "break" || comm == "b") {
    if (v[1] == "") {
      printf("USAGE: break {{{line}}\n");
    }
    else ;  // XXX: set a breakpoint
    return BREAK;
  }
  else if (comm == "step" || comm == "s") { // step実行
    return STEP;
  }
  else if (comm == "print" || comm == "p") {
    if (v[1] == "") {
      printf("USAGE: print [r0-r15/f0-f15/LO/HI]\n");
    }
    else if(!v[1].compare("LO")) {
      regs_to_show.push_back(-1);
      //regs_to_show.emplace(-1);
    }
    else if(!v[1].compare("HI")) {
      regs_to_show.push_back(-2);
      //regs_to_show.emplace(-2);
    }
    else if (!v[1].compare(0, 1, "R") || !v[1].compare(0, 1, "r")) {
      int no = std::stoi(v[1].substr(1));
      regs_to_show.push_back(no);
      //regs_to_show.emplace(no);
    }
    else if (!v[1].compare(0, 1, "F") || !v[1].compare(0, 1, "f")) {
      int no = std::stoi(v[1].substr(1));
      fregs_to_show.push_back(no);
      //fregs_to_show.emplace(no);
    }
    return PRINT;
  }
  else if (comm == "help" || comm == "h") {
    show_help();
    return HELP;
  }
  else {
    std::cerr << "Unknown command: " << s << std::endl;
    return ERR;
  }

  printf("something unexpexted has happend\n");
  return ERR;
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    printf("USAGE: %s {{filename}}\n", argv[0]);
    exit(1);
  }

  init_inst(argv[1]);

  show_help();
  putchar('\n');

  while (1) {
    enum Comm comm = read_commands();
    if (comm == NIL) break;
    else if (comm == ERR) continue;
    else if (comm == STEP) exec_inst();
    else ;  //XXX: 今はここでやることがない
    print_regs();
  }
  //test();

  puts("\nsimulator terminated");

  free(inst_reg);

  return 0;
}
