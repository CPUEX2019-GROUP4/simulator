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

#define $s (int_reg[get_rs(inst)])
#define $t (int_reg[get_rt(inst)])
#define $d (int_reg[get_rd(inst)])

enum Comm {STEP, PRINT, BREAK, HELP, NIL, ERR};

// Parameters
const char PROMPT[] = "(ryo) ";

// Prototypes
void show_help(void);
void print_regs(void);
void init_inst(char *pathname);
void exec_inst(void);
enum Comm read_commands(void);

// registers
u_int32_t *inst_reg;          // instruction register
int pc = 0;                   // program counter
int32_t int_reg[N_REG];       // int
float   float_reg[N_REG];     // float
int32_t LO, HI;               // special registers

// Meta variables
std::unordered_set<int> regs_to_show;    // 表示させるレジスタたち
std::unordered_set<int> fregs_to_show;   // (浮動小数)表示させるレジスタたち
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
  for (auto it = regs_to_show.begin(); it != regs_to_show.end(); it++) {
    printf("R%d: %d ", *it, int_reg[*it]);
  }
  //putchar('\n');
  for (auto it = fregs_to_show.begin(); it != regs_to_show.end(); it++) {
    printf("F%d: %d ", *it, float_reg[*it]);
  }
  //putchar('\n');
  // special registers
  printf("LO: %d, HI: %d\n", LO, HI);
}

/**--- read instructions from a file and save them to inst_reg ---*/
void init_inst(char *pathname)
{
  FILE *fin;

  fin = fopen(pathname, "rb");
  if (fin == NULL) {perror("fopen"); exit(1);}

  inst_reg = (u_int32_t*) malloc(LEN_INSTRUCTION * 4);  // 4 bytes per instruction
  if (inst_reg == NULL) {perror("malloc"); exit(1);}
  total_inst = fread(inst_reg, 1, LEN_INSTRUCTION, fin) / 4;
  printf("loaded %d instructions.\n", total_inst);

  if (fclose(fin)) {perror("fclose"); exit(1);}

  return;
}

u_int32_t get_opcode(u_int32_t inst) {return (inst >> 26) & 0x3f;}
u_int32_t get_rs(u_int32_t inst) {return (inst >> 21) & 0x1f;}
u_int32_t get_rt(u_int32_t inst) {return (inst >> 16) & 0x1f;}
u_int32_t get_rd(u_int32_t inst) {return (inst >> 11) & 0x1f;}
u_int32_t get_shift(u_int32_t inst) {return (inst >> 6) & 0x1f;}
u_int32_t get_funct(u_int32_t inst) {return (inst >> 0) & 0x3f;}
u_int32_t get_imm(u_int32_t inst) {return (inst >> 0) & 0x0fff;}     // 2**16
u_int32_t get_addr(u_int32_t inst) {return (inst >> 0) & 0x3ffffff;} // 2**26

/** execute single instruction */
void exec_inst(void)
{
  u_int32_t inst = inst_reg[pc];

  switch (get_opcode(inst)) {
    case 0x00:
      switch (get_funct(inst)) {
        case 0x00:  // Logical Shift Left (sll $d, $t, C) :- $d = $t << C
          $d = $t << get_imm(inst);
          printf("sll %x, %x, %x\n", get_rd(inst), get_rt(inst), get_imm(inst));
          pc++; break;
        case 0x1a:  // Divide (div $s, $t) :- LO = $s / $t. HI = $s % $t
          LO = $s / $t;
          HI = $s % $t;
          printf("div %x, %x\n", get_rs(inst), get_rt(inst));
          pc++; break;
        case 0x1b:  // Divide Unsigned (divu $s, $t) :- LO = $s / $t. HI = $s % $t
          LO = (unsigned)$s / (unsigned)$t;
          HI = (unsigned)$s % (unsigned)$t;
          printf("divu %x, %x\n", get_rs(inst), get_rt(inst));
          pc++; break;
        case 0x02:  // Shift Right Logical (0-extended) (srl $d, $t, C) :- $d = $t >> C
          $d = $t >> get_imm(inst); //XXX check if 0 is extended
          printf("srl %x, %x, %x\n", get_rd(inst), get_rt(inst), get_imm(inst));
          pc++; break;
        case 0x03:  // Shift Right Arithmetic (sign-extended) (sra $d, $t, C) :- $d = $t >> C
          $d = $t >> get_imm(inst); //XXX check if sign is extended
          printf("sra %x, %x, %x\n", get_rd(inst), get_rt(inst), get_imm(inst));
          pc++; break;
        case 0x08:  // Jump register (jr $s) :- pc = ($s)
          pc = $s;
          printf("jr %x\n", get_rs(inst));
          break;
        default:
          fprintf(stderr, "Unknown funct: %d\n", get_funct(inst));
          exit(1);
      }
      break;
    case 0x08:      // addi $t, $s, C (C: signed)
      $t = $s + get_imm(inst);
      printf("addi %x, %x, %x\n", get_rt(inst), get_rs(inst), get_imm(inst));
      pc++; break;

    default:
      fprintf(stderr, "Unknown opcode: %d\n", get_opcode(inst));
      exit(1);
  }

}

/**--- read commands from stdin ---*/
enum Comm read_commands(void)
{
  std::vector<std::string> v;
  std::string s;

  printf("%s", PROMPT);
  std::getline(std::cin, s);  // input

  // split input string by ' ' and save them to a vector v
  std::stringstream ss{s};
  std::string buf;
  while (std::getline(ss, buf, ' ')) {
    v.push_back(buf);
  }

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
      printf("USAGE: print {{{register name}}\n");
    }
    else if (!v[1].compare(0, 1, "R") || !v[1].compare(0, 1, "r")) {
      int no = std::stoi(v[1].substr(1));
      regs_to_show.emplace(no);
    }
    else if (!v[1].compare(0, 1, "F") || !v[1].compare(0, 1, "f")) {
      int no = std::stoi(v[1].substr(1));
      fregs_to_show.emplace(no);
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

void test(void)
{
  u_int32_t inst = (u_int32_t) 537001986;
  printf("t: %d\n", $t);
  $t = 3;
  printf("t: %d\n", $t);
  $t = $s + get_imm(inst);
  printf("t: %d\n", $t);
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
