#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#define BYTES_INSTRUCTION 32
#define LEN_INSTRUCTION 10000
#define N_REG 32
#define SIZE_MEM (4<<20)

#define $rd (int_reg[get_rd(inst)])
#define $ra (int_reg[get_ra(inst)])
#define $rb (int_reg[get_rb(inst)])
#define $fd (float_reg[get_rd(inst)])
#define $fa (float_reg[get_ra(inst)])
#define $fb (float_reg[get_rb(inst)])

enum Comm {STEP, PRINT, CLEAR, MONITOR, UNMONITOR, BREAK, UNBREAK, HELP, NIL, QUIT, ERR, RUN, NOP, OP};

// Parameters
const char PROMPT[] = ">> ";
int auto_display_registers = 0;

// Prototypes
void show_help(void);
void print_regs(void);
void init_inst(char *pathname);
enum Comm exec_inst(uint32_t);
enum Comm exec_inst(void);
enum Comm analyze_commands(std::string);

// registers
uint32_t *inst_reg;           // instruction register
uint32_t pc = 0;                   // program counter
int32_t int_reg[N_REG];       // int
float   float_reg[N_REG];     // float
std::array<char, SIZE_MEM> mem;           // memory
int32_t fcond_reg;

// Meta variables
std::unordered_set<int> regs_to_show;    // 表示させるレジスタたち
std::unordered_set<int> fregs_to_show;   // (浮動小数)表示させるレジスタたち
uint32_t total_inst = 0;
int dest_reg;                     // 各命令のdestination_register
std::unordered_map<int,int> ninsts;  // 命令番号に対するソースコード行番号のmap
std::unordered_map<int,int> rev_ninsts;  // ↑の逆
std::unordered_map<std::string,int> labels;  // ラベルに対するソースコード行番号のmap
int breakpoint;                   // breakpointの命令番号
std::unordered_map<int,int> regs_to_monitor;    // モニターするレジスタたち
std::unordered_map<int,float> fregs_to_monitor;    // (浮動小数)モニターするレジスタたち
std::ofstream ofs;                // OUT 命令の出力ファイル
int test_flag;                    // 出力のみ行うモード

void init(void)
{
  breakpoint = -1;
  dest_reg = -1;
}

void init_ofs(char *path)
{
  //ofs.open(path, std::ios::out | std::ios::trunc | std::ios::binary); // append はつけない
  ofs.open(path, std::ios::out | std::ios::trunc); // append はつけない
  if (ofs.fail()) {std::cerr << "File '" << path << "' could not open\n"; exit(1);}
}

void init_labels(char *path)
{
  std::string label;
  int line = 0;
  std::ifstream ifs(path);
  if (ifs.fail()) {std::cerr << "File '" << path << "' could not open\n"; exit(1);}
  while (!ifs.eof()) {
    ifs >> label >> line;
    labels.emplace(std::make_pair(label, line));
  }
}

void init_ninsts(char *path)
{
  int inst = 0;
  int line = 0;
  std::ifstream ifs(path);
  if (ifs.fail()) {std::cerr << "File '" << path << "' could not open\n"; exit(1);}
  while (!ifs.eof()) {
    ifs >> inst >> line;
    ninsts.emplace(std::make_pair(inst, line));
    rev_ninsts.emplace(std::make_pair(line, inst));
  }
}

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
  printf("step (s): \t\t\texecute the current instruction of the binary.\n");
  putchar('\n');
  printf("monitor (m) {{rn}}: \t\tstop when the value of the register changes.\n");
  putchar('\n');
  printf("unmonitor (um) {{rn}}: \t\tremove the register from monitoring list.\n");
  putchar('\n');
  printf("print (p) {{rn}}: \t\tadd the register to reserved list to show.\n"
         "                  \t\tE.g. `p r0`, `p f2`.\n");
  printf("clear (c) {{rn}}: \t\tclear provided registers from display list.\n");
  putchar('\n');
  printf("break (b) {{line/label}}: \tset a brakepoint at the line/label.\n"
         "                          \tvalid until you put `ub`.\n");
  printf("unbreak (ub): \t\t\treset the breakpoint.\n");
  putchar('\n');
  printf("run (r): \t\t\truns the program until it reaches NOP\n");
  putchar('\n');
	printf("exit/quit \t\t\tterminate the simulator\n");
  puts("-------------------");
}

int monitor(void)
{
  for (auto it = regs_to_monitor.begin(); it != regs_to_monitor.end(); it++) {
    if (int_reg[it->first] != it->second) {
      it->second = int_reg[it->first];
      return 1;
    }
  }
  for (auto it = fregs_to_monitor.begin(); it != fregs_to_monitor.end(); it++) {
    if (float_reg[it->first] != it->second) {
      it->second = float_reg[it->first];
      return 1;
    }
  }
  return 0;
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

/**--- std::string をdelimiterで分割してstd::vectorで返す ---*/
std::vector<std::string> split(std::string s, std::string delimiter, bool shrink=true)
{
  std::vector<std::string> v;
  std::string::size_type pos = 0;

  if (!delimiter.compare("")) v.push_back(s);
  else {
    while (1) {
      if (shrink) {
        while (1) {
          if (pos >= s.length()) break;
          if (s.find(delimiter, pos) == pos) pos += delimiter.length();
          else break;
        }
      }
      if (pos >= s.length()) break;
      std::string::size_type tmp = s.find(delimiter, pos);
      //if (tmp == std::string::npos) break;
      if (tmp == std::string::npos) {v.push_back(s.substr(pos, tmp-pos)); break;}
      v.push_back(s.substr(pos, tmp-pos));
      pos = tmp + delimiter.length();
    }
  }
  return v;
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
  if (!test_flag) printf("%d: ", ninsts.at(pc));
  return exec_inst(inst_reg[pc]);
}

/** execute single instruction */
enum Comm exec_inst(uint32_t inst)
{
  if (inst == 0) {printf("nop\n"); return NOP;}   // nop

  char s[256];

  switch (get_opcode(inst)) {
    case 0x00:      /* R type */
      switch (get_func(inst)) {
        case 0x20:      // add
          sprintf(s, "add r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra + $rb;
          pc++; break;
        case 0x22:      // sub
          sprintf(s, "sub r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra - $rb;
          pc++; break;
        case 0x0c:      // div2
          sprintf(s, "div2 r%d r%d\n", get_rd(inst), get_ra(inst));
          $rd = $ra > 1;
          pc++; break;
        case 0x1c:      // div10
          sprintf(s, "div10 r%d r%d\n", get_rd(inst), get_ra(inst));
          $rd = $ra / 10;
          pc++; break;
        case 0x25:      // or
          sprintf(s, "or r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra | $rb;
          pc++; break;
        case 0x2a:      // slt
          sprintf(s, "slt r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = ($ra < $rb)? 1: 0;
          pc++; break;
        case 0x04:      // sllv
          sprintf(s, "sllv r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra << $rb;
          pc++; break;
        case 0x00:      // sll
          sprintf(s, "sll r%d r%d %d\n", get_rd(inst), get_ra(inst), get_shift(inst));
          $rd = $ra << get_shift(inst);
          pc++; break;
        case 0x08:      // jr
          sprintf(s, "jr r%d\n", get_rd(inst));
          pc = $rd;
          break;
        case 0x0f:      // jalr
          sprintf(s, "jalr r%d\n", get_rd(inst));
          int_reg[31] = pc + 1;
          pc = $rd;
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
    case 0x11:      /* floating point */
      switch (get_func(inst)) {
        case 0x10:      // fneg
          sprintf(s, "fneg f%d f%d\n", get_rd(inst), get_ra(inst));
          $fd = (-1) * $fa;
          pc++; break;
        case 0x00:      // fadd
          sprintf(s, "fadd f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa + $fb;
          pc++; break;
        case 0x01:      // fsub
          sprintf(s, "fsub f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa - $fb;
          pc++; break;
        case 0x02:      // fmul
          sprintf(s, "fmul f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa * $fb;
          pc++; break;
        case 0x03:      // fdiv
          sprintf(s, "fdiv f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa / $fb;
          pc++; break;
        case 0x20:      // fclt
          sprintf(s, "fclt f%d f%d\n", get_ra(inst), get_rb(inst));
          if ($fa < $fb) fcond_reg |= 0x02;
          else fcond_reg &= 0xfffd;
          pc++; break;
        case 0x28:      // fcz
          sprintf(s, "fcz f%d \n", get_ra(inst));
          if ($fa == 0.0) fcond_reg |= 0x02;
          else fcond_reg &= 0xfffd;
          pc++; break;
        case 0x06:      // fmv
          sprintf(s, "fmv f%d f%d\n", get_rd(inst), get_ra(inst));
          $fd = $fa;
          pc++; break;
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
      sprintf(s, "addi r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $rd = $ra + get_imm_signed(inst);
      pc++; break;
    case 0x18:      // subi
      sprintf(s, "subi r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $rd = $ra - get_imm_signed(inst);
      pc++; break;
    case 0x23:      // lw
      sprintf(s, "lw r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy((char*)(&($rd)), &mem[$ra + get_imm_signed(inst)], 4);
      pc++; break;
    case 0x2b:      // sw
      sprintf(s, "sw r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy(&mem[$ra + get_imm_signed(inst)], (char*)(&($rd)), 4);
      pc++; break;
    case 0x0f:      // lui
      sprintf(s, "lui r%d %d\n", get_rd(inst), get_imm(inst));
      $rd = (get_imm(inst) << 16) & 0xff00;
      pc++; break;
    case 0x0d:      // ori
      sprintf(s, "ori r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm(inst));
      $rd = $ra | get_imm(inst);
      pc++; break;
    case 0x0a:      // slti
      sprintf(s, "slti r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $rd = ($ra < get_imm_signed(inst))? 1: 0;
      pc++; break;
    case 0x04:      // beq
      reset_bold();
      sprintf(s, "beq r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      if ($rd == $ra) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x05:      // bne
      reset_bold();
      sprintf(s, "bne r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      if ($rd != $ra) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x02:      // j
      reset_bold();
      sprintf(s, "j %d\n", get_addr(inst));
      pc = ((pc+1) & 0xf0000000) | get_addr(inst);
      break;
    case 0x03:      // jal
      reset_bold();
      sprintf(s, "jal %d\n", get_addr(inst));
      int_reg[31] = pc + 1;
      pc = ((pc+1) & 0xf0000000) | get_addr(inst);
      break;
    case 0x30:      // lwcZ
      sprintf(s, "lwcZ f%d f%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy((char*)(&($fd)), &mem[$ra + get_imm_signed(inst)], 4);
      pc++;
      break;
    case 0x38:      // swcZ
      sprintf(s, "swcZ f%d f%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy(&mem[$ra + get_imm_signed(inst)], (char*)(&($fd)), 4);
      pc++;
      break;
    case 0x13:      // bc1t
      reset_bold();
      sprintf(s, "bc1t %d\n", get_imm_signed(inst));
      if (fcond_reg&0x0002) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x15:      // bc1f
      reset_bold();
      sprintf(s, "bc1f %d\n", get_imm_signed(inst));
      if (!(fcond_reg&0x0002)) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x1c:      // ftoi
      sprintf(s, "ftoi r%d f%d\n", get_rd(inst), get_ra(inst));
      $rd = (int) $fa;
      pc++;
      break;
    case 0x1d:      // itof
      sprintf(s, "itof f%d r%d\n", get_rd(inst), get_ra(inst));
      $fd = (float) $ra;
      pc++;
      break;
    case 0x3c:      // flui
      sprintf(s, "flui f%d %d\n", get_rd(inst), get_imm(inst));
      {
        uint16_t c = get_imm(inst);
        memcpy((char*)(&($fd))+2, (char*)&c, 2);
        uint16_t z = 0;
        memcpy((char*)(&($fd)), (char*)&z, 2);
      }
      pc++;
      break;
    case 0x3d:      // fori
      sprintf(s, "fori f%d f%d %d\n", get_rd(inst), get_ra(inst), get_imm(inst));
      {
        uint16_t tmp = (uint16_t)(((uint32_t)($fa) & 0x00ff)) | get_imm(inst);
        memcpy((char*)&($fd), (char*)&tmp, 2);
      }
      pc++;
      break;
    case 0x3f:      // out
      reset_bold();
      sprintf(s, "OUT r%d %d\n", get_rd(inst), get_imm_signed(inst));
      {
        int32_t val = ($rd + get_imm_signed(inst)) % 256;
        ofs << (char)val;
        //ofs.flush();
      }
      pc++;
      break;
    default:
      reset_bold();
      fprintf(stderr, "Unknown opcode: 0x%x\n", get_opcode(inst));
      exit(1);
  }
  if (!test_flag) printf(s);
  return OP;
}

/**--- analyze commands obtained from stdin ---*/
enum Comm analyze_commands(std::string s)
{
  std::vector<std::string> v;

  v = split(s, " ");

  if (!s.compare("")) {
    std::cin.clear();
    return NIL;
  }
  if (!s.compare("exit") || !s.compare("quit")) return QUIT;

  std::string comm = v[0];

  if (comm == "break" || comm == "b") {   // breakpoint
    if (v[1] == "") {
      printf("USAGE: break {{{line/label}}\n");
    }
    else {
      try {breakpoint = rev_ninsts.at(std::stoi(v[1]));}
      catch (...) {
        try {breakpoint = labels.at(v[1]);} catch(...) {std::cerr << "Unknown label.\n"; return NIL;}
      }
      printf("breakpoint: %dth instruction\n", breakpoint);
    }
    return BREAK;
  }
  else if (comm == "unbreak" || comm == "ub") return UNBREAK;  // unbreak
  else if (comm == "step" || comm == "s") { // step実行
    return STEP;
  }
  else if (comm == "monitor" || comm == "m") {  // monitor
    if (v[1] == "") {
      printf("USAGE: monitor [r0-r15/f0-f15]\n");
    }
    else if (!v[1].compare(0, 1, "R") || !v[1].compare(0, 1, "r")) {
      int no = std::stoi(v[1].substr(1));
      regs_to_monitor.emplace(std::make_pair(no, int_reg[no]));
    }
    else if (!v[1].compare(0, 1, "F") || !v[1].compare(0, 1, "f")) {
      int no = std::stoi(v[1].substr(1));
      fregs_to_monitor.emplace(std::make_pair(no, float_reg[no]));
    }
    return MONITOR;
  }
  else if (comm == "unmonitor" || comm == "um") {
    if (v[1] == "") {
      printf("USAGE: unmonitor [r0-r15/f0-f15]\n");
    }
    else if (!v[1].compare(0, 1, "R") || !v[1].compare(0, 1, "r")) {
      int no = std::stoi(v[1].substr(1));
      regs_to_monitor.erase(no);
    }
    else if (!v[1].compare(0, 1, "F") || !v[1].compare(0, 1, "f")) {
      int no = std::stoi(v[1].substr(1));
      fregs_to_monitor.erase(no);
    }
    return UNMONITOR;  // unmonitor
  }
  else if (comm == "print" || comm == "p") {  // print
    if (v.size() == 1) {
      printf("USAGE: print [r0-r15/f0-f15]\n");
    }
    else {
      for (unsigned int i=1; i<v.size(); i++) {
        if (!v[i].compare(0, 1, "R") || !v[i].compare(0, 1, "r")) {
          int no = std::stoi(v[i].substr(1));
          regs_to_show.emplace(no);
        }
        else if (!v[i].compare(0, 1, "F") || !v[i].compare(0, 1, "f")) {
          int no = std::stoi(v[i].substr(1));
          fregs_to_show.emplace(no);
        }
        else {printf("USAGE: print [r0-r15/f0-f15]\n"); return NIL;}
      }
    }
    return PRINT;
  }
  else if (comm == "clear" || comm == "c") {  // clear
    if (v.size() == 1) {
      regs_to_show.clear();
      fregs_to_show.clear();
    }
    else {
      for (unsigned int i=1; i<v.size(); i++) {
        if (!v[i].compare(0, 1, "R") || !v[i].compare(0, 1, "r")) {
          int no = std::stoi(v[i].substr(1));
          regs_to_show.erase(no);
        }
        else if (!v[i].compare(0, 1, "F") || !v[i].compare(0, 1, "f")) {
          int no = std::stoi(v[i].substr(1));
          fregs_to_show.erase(no);
        }
        else {printf("USAGE: clear [r0-r15/f0-f15]\n"); return NIL;}
      }
    }
    return CLEAR;
  }
  else if (comm == "help" || comm == "h") {   // help
    show_help();
    return HELP;
  }
  else if (comm == "run" || comm == "r") {    // run
    return RUN;
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
  while (1) {
    if (exec_inst() == NOP) break;
  }
}

int main(int argc, char **argv)
{
  if (argc != 6) {
    printf("USAGE: %s {{binary}} {{labels}} {{insts}} {{ofs}} {{test_flag}}\n", argv[0]);
    exit(1);
  }

  if (!strcmp(argv[5], "1") || !strcmp(argv[5], "test") || !strcmp(argv[5], "true")) {
    test_flag = 1;
  }

  init();

  init_inst(argv[1]);
  init_labels(argv[2]);
  init_ninsts(argv[3]);
  init_ofs(argv[4]);

  show_help();
  putchar('\n');

  if (test_flag) test();
  else {
    std::string s;
    while (1) {
      printf("%s", PROMPT);
      if (!std::getline(std::cin, s)) break;
      enum Comm comm = analyze_commands(s);
      if (comm == NIL) continue;
      else if (comm == QUIT) break;
      else if (comm == ERR) continue;
      else if (comm == STEP) exec_inst();
      else if (comm == RUN) {
        while (1) {
          if (monitor()) break;
          if (exec_inst() == NOP) break;
          if (breakpoint < 0) continue;
          else if (pc == (unsigned)breakpoint) break;
        }
      }
      else if (comm == UNBREAK) {breakpoint = -1; continue;}
      else ;
      print_regs();
      reset_bold();
    }
  }

  puts("\nsimulator terminated");

  free(inst_reg);

  return 0;
}
