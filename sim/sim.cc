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

#define $d (int_reg[get_rd(inst)])
#define $a (int_reg[get_ra(inst)])
#define $b (int_reg[get_rb(inst)])

enum Comm {STEP, PRINT, BREAK, HELP, NIL, ERR, NOP, OP};

// Parametera
const char PROMPT[] = "(ryo) ";
constexpr int auto_display_registers = 0;

// Prototypes
void show_help(void);
void print_regs(void);
void init_inst(char *pathname);
enum Comm exec_inst(uint32_t);
enum Comm exec_inst(void);
enum Comm read_commands(void);

// registers
uint32_t *inst_reg;           // instruction register
uint32_t pc = 0;                   // program counter
int32_t int_reg[N_REG];       // int
float   float_reg[N_REG];     // float
int32_t LO, HI;               // special registers
std::array<char, SIZE_MEM> mem;           // memory

// Meta variables
std::vector<int> regs_to_show;    // 表示させるレジスタたち
std::vector<int> fregs_to_show;   // (浮動小数)表示させるレジスタたち
uint32_t total_inst = 0;
int dest_reg;                     // 各命令のdestination_register

/** for register display */
void set_bold(int i)
{
  dest_reg = i;
}

/** for register display */
void reset_bold(void)
{
  dest_reg = -100;
}

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
    if (it == dest_reg) printf("\x1b[1m");
    printf("\t%d: r%d = %d\n", ++count, it, int_reg[it]);
    if (it == dest_reg) printf("\x1b[0m");
  }
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
uint32_t get_rd(uint32_t inst) {return (inst >> 21) & 0x1f;}
uint32_t get_ra(uint32_t inst) {return (inst >> 16) & 0x1f;}
uint32_t get_rb(uint32_t inst) {return (inst >> 11) & 0x1f;}
uint32_t get_shift(uint32_t inst) {return (inst >> 6) & 0x1f;}
int32_t get_shift_signed(uint32_t inst) {return (inst & 0x3ff) - (inst & 0x400);}
uint32_t get_func(uint32_t inst) {return (inst >> 0) & 0x3f;}
uint32_t get_imm(uint32_t inst) {return (inst >> 0) & 0xffff;}
int32_t get_imm_signed(uint32_t inst) {return (inst & 0x7fff) - (inst & 0x8000);}
uint32_t get_addr(uint32_t inst) {return (inst >> 0) & 0x3ffffff;}

/** execute single instruction */
enum Comm exec_inst(void)
{
  if (!(0 <= pc && pc < total_inst)) {
    printf("pc out of index. Abort. %d of %d\n", pc, total_inst);
    exit(1);
  }
  printf("%d: ", pc);
  return exec_inst(inst_reg[pc]);
}

/** execute single instruction */
enum Comm exec_inst(uint32_t inst)
{
  if (inst == 0) {printf("nop\n"); return NOP;}   // nop

  set_bold(get_rd(inst));

  switch (get_opcode(inst)) {
    case 0x00:      /* R type */
      switch (get_func(inst)) {
        case 0x20:      // add
          printf("add r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $d = $a + $b;
          pc++; break;
        case 0x22:      // sub
          printf("sub r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $d = $a - $b;
          pc++; break;
        case 0x25:      // or
          printf("or r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $d = $a | $b;
          pc++; break;
        case 0x2a:      // slt
          printf("slt r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $d = ($a < $b)? 1: 0;
          pc++; break;
        case 0x04:      // sllv
          printf("sllv r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $d = $a << $b;
          pc++; break;
        case 0x00:      // sll
          printf("sll r%d r%d %d\n", get_rd(inst), get_ra(inst), get_shift(inst));
          $d = $a << get_shift(inst);
          pc++; break;
        case 0x08:      // jr
          reset_bold();
          printf("jr r%d\n", get_rd(inst));
          pc = $d;
          break;
        default:
          reset_bold();
          printf("Unknown funct: 0x%x.\n", get_func(inst));
          printf("opcode: 0x%d, rd: %d, ra: %d, rb: %d, shift: %d, func: 0x%x\n",
                 get_opcode(inst), get_rd(inst), get_ra(inst), get_rb(inst),
                 get_shift(inst), get_func(inst));
          puts("Abort.");
          exit(1);
      }
      break;
    case 0x08:      // addi
      printf("addi r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $d = $a + get_imm_signed(inst);
      pc++; break;
    case 0x18:      // subi
      printf("subi r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $d = $a - get_imm_signed(inst);
      pc++; break;
    case 0x23:      // lw
      printf("lw r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      copy((char*)(&($d)), &mem[$a + get_imm_signed(inst)], 4);
      pc++; break;
    case 0x2b:      // sw
      reset_bold();
      printf("sw r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      copy(&mem[$a + get_imm_signed(inst)], (char*)(&($d)), 4);
      pc++; break;
    case 0x0f:      // lui
      printf("lui r%d %d\n", get_rd(inst), get_imm(inst));
      $d = get_imm(inst) << 16;
      pc++; break;
    case 0x0d:      // ori
      printf("ori r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm(inst));
      $d = $a | get_imm(inst);
      pc++; break;
    case 0x0a:      // slti
      printf("slti r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $d = ($a < get_imm_signed(inst))? 1: 0;
      pc++; break;
    case 0x04:      // beq
      reset_bold();
      printf("beq r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      if ($d == $a) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x05:      // bne
      reset_bold();
      printf("bne r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      if ($d != $a) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x02:      // j
      reset_bold();
      printf("j %d\n", get_addr(inst));
      pc = ((pc+1) & 0xf0000000) | get_addr(inst);
      break;
    case 0x03:      // jal
      reset_bold();
      printf("jal %d\n", get_addr(inst));
      int_reg[31] = pc + 1;
      pc = ((pc+1) & 0xf0000000) | get_addr(inst);
      break;
    default:
      reset_bold();
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
    else if (!v[1].compare(0, 1, "R") || !v[1].compare(0, 1, "r")) {
      int no = std::stoi(v[1].substr(1));
      regs_to_show.push_back(no);
    }
    else if (!v[1].compare(0, 1, "F") || !v[1].compare(0, 1, "f")) {
      int no = std::stoi(v[1].substr(1));
      fregs_to_show.push_back(no);
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
  uint32_t inst = 2351300604;
  printf("%d\n", get_imm_signed(inst));
  printf("%d\n", get_imm(inst));
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
    int ret = exec_inst();
    if (ret == NOP) break;
  }

  while (1) {
    enum Comm comm = read_commands();
    if (comm == NIL) break;
    else if (comm == ERR) continue;
    else if (comm == STEP) exec_inst();
    else ;  //XXX: 今はここでやることがない
    print_regs();
    reset_bold();
  }
  //test();

  puts("\nsimulator terminated");

  free(inst_reg);

  return 0;
}
