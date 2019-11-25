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
#include <cmath>
#include <iomanip>
#include <locale>

#define BYTES_INSTRUCTION 4
#define LEN_INSTRUCTION 200000
#define N_REG 32
#define SIZE_MEM (2<<20)

#define $rd (int_reg[get_rd(inst)])
#define $ra (int_reg[get_ra(inst)])
#define $rb (int_reg[get_rb(inst)])
#define $fd (float_reg[get_rd(inst)])
#define $fa (float_reg[get_ra(inst)])
#define $fb (float_reg[get_rb(inst)])

enum Comm {STEP, PRINT, CLEAR, MONITOR, UNMONITOR, BREAK, UNBREAK, HELP, NIL, QUIT, ERR, RUN, FAST, NOP, OP};

// Parameters
const char PROMPT[] = ">> ";
int auto_display_registers = 0;

// Prototypes
void show_help(void);
void print_regs(void);
void init_inst(char *pathname);
Comm exec_inst(uint32_t);
Comm exec_inst(void);
//enum Comm analyze_commands(std::string);

// registers
uint32_t *inst_reg;           // instruction register
uint32_t pc = 0;                   // program counter
int32_t int_reg[N_REG];       // int
float   float_reg[N_REG];     // float
std::array<unsigned char, SIZE_MEM> mem;           // memory
int32_t fcond_reg;

// Meta variables
std::unordered_set<int> regs_to_show;    // 表示させるレジスタたち
std::unordered_set<int> fregs_to_show;   // (浮動小数)表示させるレジスタたち
std::unordered_set<long> address_to_show;   // メモリアドレス
std::unordered_set<long> intaddress_to_show;   // メモリアドレス(int)
std::unordered_set<long> fltaddress_to_show;   // メモリアドレス(flt)
uint32_t total_inst = 0;
int dest_reg;                     // 各命令のdestination_register
std::unordered_map<int,int> ninsts;  // 命令番号に対するソースコード行番号のmap
std::unordered_map<int,int> rev_ninsts;  // ↑の逆
std::unordered_map<std::string,int> labels;  // ラベルに対するソースコード行番号のmap
int breakpoint;                   // breakpointの命令番号
std::unordered_map<int,int> regs_to_monitor;    // モニターするレジスタたち
std::unordered_map<int,float> fregs_to_monitor;    // (浮動小数)モニターするレジスタたち
std::unordered_map<long,unsigned char> address_to_monitor;    // モニターするアドレス
std::ofstream ofs;                // OUT 命令の出力ファイル
int test_flag;                    // 出力のみ行うモード
FILE *fin;                // IN 命令のファイル
long total_executed = 0;          // 実行された総演算命令数
long r29_max, r30_max;

union bits {
  float f;
  uint32_t ui32;
  struct {
    uint16_t lo;
    uint16_t hi;
  } lohi;
} b;

//XXX: fpuのcopy (for performance) (as of 25 Nov)
#define MINPREC 6
#define MOUTPREC 6
#define SQRT_LOOP_COUNT 2
#define FINV_LOOP_COUNT 2

#define MUSE(prec) (0x00800000 - (1 << (23 - prec)))

uint32_t sqrt_init_m(uint32_t emod2, uint32_t m) {
    b.ui32 = (m & MUSE(MINPREC)) | (emod2 ? 0x3f800000 : 0x40000000);
    b.f = sqrtf(b.f);
    uint32_t m_ = b.ui32 & MUSE(MOUTPREC);
    return m_;
}

uint32_t sqrt_init_u(float f) {
    b.f = f;
    uint32_t u = b.ui32;
    uint32_t s = u & 0x80000000, e = (u >> 23) & 0x000000ff, m = u & 0x007fffff;
    if (e == 0) return 0;
    uint32_t m_ = sqrt_init_m(e & 1, m);
    uint32_t e_ = ((e - 1) >> 1) + 64;
    uint32_t u_ = s | (e_ << 23) | m_;
    return u_;
}

float sqrt_init(float f) {
    b.ui32 = sqrt_init_u(f);
    return b.f;
}

uint32_t finv_init_m(uint32_t m) {
    b.ui32 = (m & MUSE(MINPREC)) | 0x3f800000;
    b.f = 2.0f / b.f;
    uint32_t m_ = b.ui32 & MUSE(MOUTPREC);
    return m_;
}

uint32_t finv_init_u(float f) {
    b.f = f;
    uint32_t u = b.ui32;
    uint32_t s = u & 0x80000000, e = (u >> 23) & 0x000000ff, m = u & 0x007fffff;
    if (e == 0) return 0;
    uint32_t m_ = finv_init_m(m);
    uint32_t e_ = ((253 - e) & 0x000000ff) + (m & MUSE(MINPREC) ? 0 : 1);
    uint32_t u_ = s | (e_ << 23) | m_;
    return u_;
}

float finv_init(float f) {
    b.ui32 = finv_init_u(f);
    return b.f;
}

float mfinv(float x, float init) {
    float t = init;
    for (int i = 0; i < FINV_LOOP_COUNT; i++) {
        t = t * (2 - x * t);
    }
    return t;
}
//XXX: end of copy

template<class T>
std::string FormatWithCommas(T value)
{
  std::stringstream ss;
  ss.imbue(std::locale(""));
  ss << std::fixed << value;
  return ss.str();
}

void init(void)
{
  breakpoint = -1;
  dest_reg = -1;
}

void init_ofs(char *path)
{
  ofs.open(path, std::ios::out | std::ios::trunc); // append はつけない
  if (ofs.fail()) {std::cerr << "(ofs)File '" << path << "' could not be opened\n"; exit(1);}
}

void init_fin(char *path)
{
  fin = fopen(path, "r");
  if (!fin) {std::cerr << "(fin)File '" << path << "' could not be opened\n"; exit(1);}
}

void init_labels(char *path)
{
  std::string label;
  int line = 0;
  std::ifstream ifs(path);
  if (ifs.fail()) {std::cerr << "(ifs)File '" << path << "' could not open\n"; exit(1);}
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
  printf("at (a): \t\t\tprints the current pc and line.\n");
  putchar('\n');
  printf("step (s): \t\t\texecutes the current instruction of the binary.\n");
  putchar('\n');
  printf("monitor (m) {{rn}}: \t\thalts when the value of the register changes.\n");
  putchar('\n');
  printf("unmonitor (um) {{rn}}: \t\tremoves the register from monitoring list.\n");
  putchar('\n');
  printf("print (p) {{rn}}: \t\tadds the register to reserved list to show.\n"
         "                  \t\tE.g. `p r0`, `p f2`.\n");
  printf("clear (c) {{rn}}: \t\tclears provided registers from display list.\n");
  putchar('\n');
  printf("break (b) {{line/label}}: \tsets a brakepoint at the line/label.\n"
         "                          \tvalid until you put `ub`.\n");
  printf("unbreak (ub): \t\t\tresets the breakpoint.\n");
  putchar('\n');
  printf("run (r) {{n}}: \t\t\truns the program until it reaches NOP, \n"
         "               \t\t\tor at the nth instruction if given\n");
  printf("fast (f) {{n}}: \t\truns the program in silent mode until it reaches NOP, \n"
         "                \t\tor at the nth instruction if given\n");
	printf("exit/quit \t\t\tterminates the simulator\n");
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
  for (auto it = address_to_monitor.begin(); it != address_to_monitor.end(); it++) {
    if (mem.at(it->first) != it->second) {
      it->second = mem[it->first];
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
    printf("\t%d: f%d = %f\n",  ++count, it, float_reg[it]);
  }
  for (auto it : address_to_show) {
    printf("\t%d: M[%ld] = %d\n", ++count, it, mem.at(it));
  }
  for (auto it : intaddress_to_show) {
    memcpy((unsigned char*)&b.ui32, (unsigned char*)&mem.at(it), 4);
    printf("\t%d: M[%ld:%ld](:int) = %d\n", ++count, it, it+3, b.ui32);
  }
  for (auto it : fltaddress_to_show) {
    memcpy((unsigned char*)&b.f, (unsigned char*)&mem.at(it), 4);
    printf("\t%d: M[%ld:%ld](:flt) = %f\n", ++count, it, it+3, b.f);
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

  inst_reg = (uint32_t*) malloc(LEN_INSTRUCTION * BYTES_INSTRUCTION);
  if (inst_reg == NULL) {perror("malloc"); exit(1);}
  total_inst = fread(inst_reg, BYTES_INSTRUCTION, LEN_INSTRUCTION, fin);
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
uint16_t get_imm(uint32_t inst) {return (inst >> 0) & 0xffff;}
int16_t get_imm_signed(uint32_t inst) {return (inst & 0x7fff) - (inst & 0x8000);}
uint32_t get_addr(uint32_t inst) {return (inst >> 0) & 0x3ffffff;}

/** execute single instruction */
Comm exec_inst(void)
{
  if (!(0 <= pc && pc < total_inst)) {
    printf("pc out of index. Abort. %d of %d\n", pc, total_inst);
    exit(1);
  }
  if (!test_flag) printf("%d: ", ninsts.at(pc));
  return exec_inst(inst_reg[pc]);
}

Comm exec_inst_silent(void)
{
  while (1) {
    if (monitor()) return MONITOR;
    if ((breakpoint >= 0) && (pc == (unsigned)breakpoint)) return BREAK;

    if (!(0 <= pc && pc < total_inst)) {
      printf("pc out of index. Abort. %d of %d\n", pc, total_inst);
      exit(1);
    }
    uint32_t inst = inst_reg[pc];

    total_executed++;

    //if (int_reg[29] > r29_max) r29_max = int_reg[29];
    //if (int_reg[30] > r30_max) r30_max = int_reg[30];

    if (inst == 0) {printf("nop\n"); return NOP;}   // nop

    switch (get_opcode(inst)) {
      case 0x00:      /* R type */
        switch (get_func(inst)) {
          case 0x20:      // add
            $rd = $ra + $rb;
            pc++; break;
          case 0x22:      // sub
            $rd = $ra - $rb;
            pc++; break;
          case 0x0c:      // div2
            $rd = $ra >> 1;
            pc++; break;
          case 0x1c:      // div10
            $rd = $ra / 10;
            pc++; break;
          case 0x25:      // or
            $rd = $ra | $rb;
            pc++; break;
          case 0x2a:      // slt
            $rd = ($ra < $rb)? 1: 0;
            pc++; break;
          case 0x04:      // sllv
            $rd = $ra << $rb;
            pc++; break;
          case 0x00:      // sll
            $rd = $ra << get_shift(inst);
            pc++; break;
          case 0x08:      // jr
            pc = $rd;
            break;
          case 0x0f:      // jalr
            int_reg[31] = pc + 1;
            pc = $rd;
            break;
          default:
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
            $fd = (-1) * $fa;
            pc++; break;
          case 0x00:      // fadd
            $fd = $fa + $fb;
            pc++; break;
          case 0x01:      // fsub
            $fd = $fa - $fb;
            pc++; break;
          case 0x02:      // fmul
            $fd = $fa * $fb;
            pc++; break;
          case 0x03:      // fdiv
            $fd = $fa / $fb;
            pc++; break;
          case 0x20:      // fclt
            if ($fa < $fb) fcond_reg |= 0x02;
            else fcond_reg &= 0xfffd;
            pc++; break;
          case 0x28:      // fcz
            if ($fa == 0.0) fcond_reg |= 0x02;
            else fcond_reg &= 0xfffd;
            pc++; break;
          case 0x06:      // fmv
            $fd = $fa;
            pc++; break;
          case 0x30:      // sqrt_init
            $fd = sqrt_init($fa);
            pc++; break;
          case 0x38:      // finv_init
            $fd = finv_init($fa);
            pc++; break;
          default:
            printf("Unknown funct: 0x%x.\n", get_func(inst));
            printf("opcode: 0x%d, rd: %d, ra: %d, rb: %d, shift: %d, func: 0x%x\n",
                   get_opcode(inst), get_rd(inst), get_ra(inst), get_rb(inst),
                   get_shift(inst), get_func(inst));
            puts("Abort.");
            exit(1);
        }
        break;
      case 0x08:      // addi
        $rd = $ra + get_imm_signed(inst);
        pc++; break;
      case 0x23:      // lw
        memcpy((unsigned char*)(&($rd)), &mem.at($ra + get_imm_signed(inst)), 4);
        pc++; break;
      case 0x2b:      // sw
        memcpy(&mem.at($ra + get_imm_signed(inst)), (unsigned char*)(&($rd)), 4);
        pc++; break;
      case 0x0f:      // lui
        b.lohi.hi = get_imm(inst);
        b.lohi.lo = 0;
        $rd = b.ui32;
        pc++; break;
      case 0x0d:      // ori
        $rd = $ra | get_imm(inst);
        pc++; break;
      case 0x0a:      // slti
        $rd = ($ra < get_imm_signed(inst))? 1: 0;
        pc++; break;
      case 0x04:      // beq
        if ($rd == $ra) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x05:      // bne
        if ($rd != $ra) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x02:      // j
        pc = ((pc+1) & 0xf0000000) | get_addr(inst);
        break;
      case 0x03:      // jal
        int_reg[31] = pc + 1;
        pc = ((pc+1) & 0xf0000000) | get_addr(inst);
        break;
      case 0x30:      // lwcZ
        memcpy((unsigned char*)(&($fd)), &mem.at($ra + get_imm_signed(inst)), 4);
        pc++;
        break;
      case 0x38:      // swcZ
        memcpy(&mem.at($ra + get_imm_signed(inst)), (unsigned char*)(&($fd)), 4);
        pc++;
        break;
      case 0x13:      // bc1t
        if (fcond_reg&0x0002) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x15:      // bc1f
        if (!(fcond_reg&0x0002)) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x1c:      // ftoi
        $rd = (int) ($fa);
        pc++;
        break;
      case 0x1d:      // itof
        $fd = (float) $ra;
        pc++;
        break;
      case 0x3c:      // flui
        b.lohi.hi = get_imm(inst);
        b.lohi.lo = 0;
        $fd = b.f;
        pc++;
        break;
      case 0x3d:      // fori
        b.f = $fa;
        b.lohi.lo |= get_imm(inst);
        $fd = b.f;
        pc++;
        break;
      case 0x3f:      // out
        ofs << (char)(($rd + get_imm_signed(inst)) % 256);
        pc++;
        break;
      case 0x18:      // inint
        if (fread((char*)&($rd), 4, 1, fin) != 1) {std::cerr << "fread\n"; exit(1);}
        pc++;
        break;
      case 0x19:      // inflt
        if (fread((char*)&($fd), 4, 1, fin) != 1) {std::cerr << "fread\n"; exit(1);}
        pc++;
        break;
      default:
        fprintf(stderr, "Unknown opcode: 0x%x\n", get_opcode(inst));
        exit(1);
    }
  }

  return OP;
}

Comm exec_inst_silent(long max_count)
{
  while (max_count-- > 0) {
    if (!(0 <= pc && pc < total_inst)) {
      printf("pc out of index. Abort. %d of %d\n", pc, total_inst);
      exit(1);
    }
    uint32_t inst = inst_reg[pc];

    total_executed++;

    //if (int_reg[29] > r29_max) r29_max = int_reg[29];
    //if (int_reg[31] > r30_max) r30_max = int_reg[31];

    if (inst == 0) {printf("nop\n"); return NOP;}   // nop

    switch (get_opcode(inst)) {
      case 0x00:      /* R type */
        switch (get_func(inst)) {
          case 0x20:      // add
            $rd = $ra + $rb;
            pc++; break;
          case 0x22:      // sub
            $rd = $ra - $rb;
            pc++; break;
          case 0x0c:      // div2
            $rd = $ra >> 1;
            pc++; break;
          case 0x1c:      // div10
            $rd = $ra / 10;
            pc++; break;
          case 0x25:      // or
            $rd = $ra | $rb;
            pc++; break;
          case 0x2a:      // slt
            $rd = ($ra < $rb)? 1: 0;
            pc++; break;
          case 0x04:      // sllv
            $rd = $ra << $rb;
            pc++; break;
          case 0x00:      // sll
            $rd = $ra << get_shift(inst);
            pc++; break;
          case 0x08:      // jr
            pc = $rd;
            break;
          case 0x0f:      // jalr
            int_reg[31] = pc + 1;
            pc = $rd;
            break;
          default:
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
            $fd = (-1) * $fa;
            pc++; break;
          case 0x00:      // fadd
            $fd = $fa + $fb;
            pc++; break;
          case 0x01:      // fsub
            $fd = $fa - $fb;
            pc++; break;
          case 0x02:      // fmul
            $fd = $fa * $fb;
            pc++; break;
          case 0x03:      // fdiv
            $fd = $fa / $fb;
            pc++; break;
          case 0x20:      // fclt
            if ($fa < $fb) fcond_reg |= 0x02;
            else fcond_reg &= 0xfffd;
            pc++; break;
          case 0x28:      // fcz
            if ($fa == 0.0) fcond_reg |= 0x02;
            else fcond_reg &= 0xfffd;
            pc++; break;
          case 0x06:      // fmv
            $fd = $fa;
            pc++; break;
          case 0x30:      // sqrt_init
            $fd = sqrt_init($fa);
            pc++; break;
          case 0x38:      // finv_init
            $fd = finv_init($fa);
            pc++; break;
          default:
            printf("Unknown funct: 0x%x.\n", get_func(inst));
            printf("opcode: 0x%d, rd: %d, ra: %d, rb: %d, shift: %d, func: 0x%x\n",
                   get_opcode(inst), get_rd(inst), get_ra(inst), get_rb(inst),
                   get_shift(inst), get_func(inst));
            puts("Abort.");
            exit(1);
        }
        break;
      case 0x08:      // addi
        $rd = $ra + get_imm_signed(inst);
        pc++; break;
      case 0x23:      // lw
        memcpy((unsigned char*)(&($rd)), &mem.at($ra + get_imm_signed(inst)), 4);
        pc++; break;
      case 0x2b:      // sw
        memcpy(&mem.at($ra + get_imm_signed(inst)), (unsigned char*)(&($rd)), 4);
        pc++; break;
      case 0x0f:      // lui
        b.lohi.hi = get_imm(inst);
        b.lohi.lo = 0;
        $rd = b.ui32;
        pc++; break;
      case 0x0d:      // ori
        $rd = $ra | get_imm(inst);
        pc++; break;
      case 0x0a:      // slti
        $rd = ($ra < get_imm_signed(inst))? 1: 0;
        pc++; break;
      case 0x04:      // beq
        if ($rd == $ra) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x05:      // bne
        if ($rd != $ra) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x02:      // j
        pc = ((pc+1) & 0xf0000000) | get_addr(inst);
        break;
      case 0x03:      // jal
        int_reg[31] = pc + 1;
        pc = ((pc+1) & 0xf0000000) | get_addr(inst);
        break;
      case 0x30:      // lwcZ
        memcpy((unsigned char*)(&($fd)), &mem.at($ra + get_imm_signed(inst)), 4);
        pc++;
        break;
      case 0x38:      // swcZ
        memcpy(&mem.at($ra + get_imm_signed(inst)), (unsigned char*)(&($fd)), 4);
        pc++;
        break;
      case 0x13:      // bc1t
        if (fcond_reg&0x0002) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x15:      // bc1f
        if (!(fcond_reg&0x0002)) pc += 1 + get_imm_signed(inst);
        else pc++;
        break;
      case 0x1c:      // ftoi
        $rd = (int) ($fa);
        pc++;
        break;
      case 0x1d:      // itof
        $fd = (float) $ra;
        pc++;
        break;
      case 0x3c:      // flui
        b.lohi.hi = get_imm(inst);
        b.lohi.lo = 0;
        $fd = b.f;
        pc++;
        break;
      case 0x3d:      // fori
        b.f = $fa;
        b.lohi.lo |= get_imm(inst);
        $fd = b.f;
        pc++;
        break;
      case 0x3f:      // out
        ofs << (char)(($rd + get_imm_signed(inst)) % 256);
        pc++;
        break;
      case 0x18:      // inint
        if (fread((char*)&($rd), 4, 1, fin) != 1) {std::cerr << "fread\n"; exit(1);}
        pc++;
        break;
      case 0x19:      // inflt
        if (fread((char*)&($fd), 4, 1, fin) != 1) {std::cerr << "fread\n"; exit(1);}
        pc++;
        break;
      default:
        fprintf(stderr, "Unknown opcode: 0x%x\n", get_opcode(inst));
        exit(1);
    }
  }

  return OP;
}

/** execute single instruction */
Comm exec_inst(uint32_t inst)
{
  total_executed++;
  //if (int_reg[29] > r29_max) r29_max = int_reg[29];
  //if (int_reg[31] > r30_max) r30_max = int_reg[31];

  if (inst == 0) {printf("nop\n"); return NOP;}   // nop

  //char s[256];

  switch (get_opcode(inst)) {
    case 0x00:      /* R type */
      switch (get_func(inst)) {
        case 0x20:      // add
          if (!test_flag) printf("add r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra + $rb;
          pc++; break;
        case 0x22:      // sub
          if (!test_flag) printf("sub r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra - $rb;
          pc++; break;
        case 0x0c:      // div2
          if (!test_flag) printf("div2 r%d r%d\n", get_rd(inst), get_ra(inst));
          $rd = $ra >> 1;
          pc++; break;
        case 0x1c:      // div10
          if (!test_flag) printf("div10 r%d r%d\n", get_rd(inst), get_ra(inst));
          $rd = $ra / 10;
          pc++; break;
        case 0x25:      // or
          if (!test_flag) printf("or r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra | $rb;
          pc++; break;
        case 0x2a:      // slt
          if (!test_flag) printf("slt r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = ($ra < $rb)? 1: 0;
          pc++; break;
        case 0x04:      // sllv
          if (!test_flag) printf("sllv r%d r%d r%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $rd = $ra << $rb;
          pc++; break;
        case 0x00:      // sll
          if (!test_flag) printf("sll r%d r%d %d\n", get_rd(inst), get_ra(inst), get_shift(inst));
          $rd = $ra << get_shift(inst);
          pc++; break;
        case 0x08:      // jr
          if (!test_flag) printf("jr r%d\n", get_rd(inst));
          pc = $rd;
          break;
        case 0x0f:      // jalr
          if (!test_flag) printf("jalr r%d\n", get_rd(inst));
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
          if (!test_flag) printf("fneg f%d f%d\n", get_rd(inst), get_ra(inst));
          $fd = (-1) * $fa;
          pc++; break;
        case 0x00:      // fadd
          if (!test_flag) printf("fadd f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa + $fb;
          pc++; break;
        case 0x01:      // fsub
          if (!test_flag) printf("fsub f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa - $fb;
          pc++; break;
        case 0x02:      // fmul
          if (!test_flag) printf("fmul f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa * $fb;
          pc++; break;
        case 0x03:      // fdiv
          if (!test_flag) printf("fdiv f%d f%d f%d\n", get_rd(inst), get_ra(inst), get_rb(inst));
          $fd = $fa / $fb;
          pc++; break;
        case 0x20:      // fclt
          if (!test_flag) printf("fclt f%d f%d\n", get_ra(inst), get_rb(inst));
          if ($fa < $fb) fcond_reg |= 0x02;
          else fcond_reg &= 0xfffd;
          pc++; break;
        case 0x28:      // fcz
          if (!test_flag) printf("fcz f%d \n", get_ra(inst));
          if ($fa == 0.0) fcond_reg |= 0x02;
          else fcond_reg &= 0xfffd;
          pc++; break;
        case 0x06:      // fmv
          if (!test_flag) printf("fmv f%d f%d\n", get_rd(inst), get_ra(inst));
          $fd = $fa;
          pc++; break;
        case 0x30:      // sqrt_init
          if (!test_flag) printf("sqrt_init f%d f%d\n", get_rd(inst), get_ra(inst));
          $fd = sqrt_init($fa);
          pc++; break;
        case 0x38:      // finv_init
          if (!test_flag) printf("finv_init f%d f%d\n", get_rd(inst), get_ra(inst));
          $fd = finv_init($fa);
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
      if (!test_flag) printf("addi r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $rd = $ra + get_imm_signed(inst);
      pc++; break;
    case 0x23:      // lw
      if (!test_flag) printf("lw r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy((unsigned char*)(&($rd)), &mem.at($ra + get_imm_signed(inst)), 4);
      pc++; break;
    case 0x2b:      // sw
      if (!test_flag) printf("sw r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy(&mem.at($ra + get_imm_signed(inst)), (unsigned char*)(&($rd)), 4);
      pc++; break;
    case 0x0f:      // lui
      if (!test_flag) printf("lui r%d %d\n", get_rd(inst), get_imm(inst));
      b.lohi.hi = get_imm(inst);
      b.lohi.lo = 0;
      $rd = b.ui32;
      pc++; break;
    case 0x0d:      // ori
      if (!test_flag) printf("ori r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm(inst));
      $rd = $ra | get_imm(inst);
      pc++; break;
    case 0x0a:      // slti
      if (!test_flag) printf("slti r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      $rd = ($ra < get_imm_signed(inst))? 1: 0;
      pc++; break;
    case 0x04:      // beq
      reset_bold();
      if (!test_flag) printf("beq r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      if ($rd == $ra) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x05:      // bne
      reset_bold();
      if (!test_flag) printf("bne r%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      if ($rd != $ra) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x02:      // j
      reset_bold();
      if (!test_flag) printf("j %d\n", get_addr(inst));
      pc = ((pc+1) & 0xf0000000) | get_addr(inst);
      break;
    case 0x03:      // jal
      reset_bold();
      if (!test_flag) printf("jal %d\n", get_addr(inst));
      int_reg[31] = pc + 1;
      pc = ((pc+1) & 0xf0000000) | get_addr(inst);
      break;
    case 0x30:      // lwcZ
      if (!test_flag) printf("lwcZ f%d f%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy((unsigned char*)(&($fd)), &mem.at($ra + get_imm_signed(inst)), 4);
      pc++;
      break;
    case 0x38:      // swcZ
      if (!test_flag) printf("swcZ f%d r%d %d\n", get_rd(inst), get_ra(inst), get_imm_signed(inst));
      memcpy(&mem.at($ra + get_imm_signed(inst)), (unsigned char*)(&($fd)), 4);
      pc++;
      break;
    case 0x13:      // bc1t
      reset_bold();
      if (!test_flag) printf("bc1t %d\n", get_imm_signed(inst));
      if (fcond_reg&0x0002) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x15:      // bc1f
      reset_bold();
      if (!test_flag) printf("bc1f %d\n", get_imm_signed(inst));
      if (!(fcond_reg&0x0002)) pc += 1 + get_imm_signed(inst);
      else pc++;
      break;
    case 0x1c:      // ftoi
      if (!test_flag) printf("ftoi r%d f%d\n", get_rd(inst), get_ra(inst));
      $rd = (int) ($fa);
      pc++;
      break;
    case 0x1d:      // itof
      if (!test_flag) printf("itof f%d r%d\n", get_rd(inst), get_ra(inst));
      $fd = (float) $ra;
      pc++;
      break;
    case 0x3c:      // flui
      if (!test_flag) printf("flui f%d %d\n", get_rd(inst), get_imm(inst));
      b.lohi.hi = get_imm(inst);
      b.lohi.lo = 0;
      $fd = b.f;
      pc++;
      break;
    case 0x3d:      // fori
      if (!test_flag) printf("fori f%d f%d %d\n", get_rd(inst), get_ra(inst), get_imm(inst));
      b.f = $fa;
      b.lohi.lo |= get_imm(inst);
      $fd = b.f;
      pc++;
      break;
    case 0x3f:      // out
      reset_bold();
      if (!test_flag) printf("out r%d %d\n", get_rd(inst), get_imm_signed(inst));
      {
        int32_t val = ($rd + get_imm_signed(inst)) % 256;
        ofs << (char)val;
      }
      pc++;
      break;
    case 0x18:      // inint
      if (!test_flag) printf("inint r%d\n", get_rd(inst));
      {
        int cc = fread((char*)&($rd), 4, 1, fin);
        if (cc != 1) {std::cerr << "fread\n" << cc; exit(1);}
      }
      pc++;
      break;
    case 0x19:      // inflt
      if (!test_flag) printf("inflt f%d\n", get_rd(inst));
      {
        int cc = fread((char*)&($fd), 4, 1, fin);
        if (cc != 1) {std::cerr << "fread\n" << cc; exit(1);}
      }
      pc++;
      break;
    default:
      reset_bold();
      fprintf(stderr, "Unknown opcode: 0x%x\n", get_opcode(inst));
      exit(1);
  }
  //if (!test_flag) printf(s);
  return OP;
}

/**--- analyze commands obtained from stdin ---*/
std::pair<Comm, long> analyze_commands(std::string s)
{
  std::pair<Comm, long> ret;
  ret.second = 0;

  std::vector<std::string> v;

  v = split(s, " ");

  if (!s.compare("")) {
    std::cin.clear();
    ret.first = NIL;
    return ret;
  }
  else if (!s.compare("exit") || !s.compare("quit")) {ret.first = QUIT; return ret;}

  std::string comm = v[0];

  if (comm == "break" || comm == "b") {   // breakpoint
    if (v[1] == "") {
      printf("USAGE: break {{{line/label}}\n");
    }
    else {
      try {breakpoint = rev_ninsts.at(std::stoi(v[1]));}
      catch (...) {
        try {breakpoint = labels.at(v[1]);} catch(...) {std::cerr << "Unknown label.\n"; ret.first = NIL;}
      }
      printf("breakpoint: %dth instruction\n", breakpoint);
    }
    ret.first = BREAK;
  }
  else if (comm == "unbreak" || comm == "ub") ret.first = UNBREAK;  // unbreak
  else if (comm == "step" || comm == "s") { // step
    ret.first = STEP;
  }
  else if (comm == "monitor" || comm == "m") {  // monitor
    if (v[1] == "") {
      printf("USAGE: monitor [r0-r15/f0-f15/m{{address}}]\n");
    }
    else if (!v[1].compare(0, 1, "R") || !v[1].compare(0, 1, "r")) {
      int no = std::stoi(v[1].substr(1));
      regs_to_monitor.emplace(std::make_pair(no, int_reg[no]));
    }
    else if (!v[1].compare(0, 1, "F") || !v[1].compare(0, 1, "f")) {
      int no = std::stoi(v[1].substr(1));
      fregs_to_monitor.emplace(std::make_pair(no, float_reg[no]));
    }
    else if (!v[1].compare(0, 1, "M") || !v[1].compare(0, 1, "m")) {
      long no = std::stol(v[1].substr(1));
      try {
        address_to_monitor.emplace(std::make_pair(no, mem.at(no)));
      }
      catch (...) {
        std::cerr << "invalid address\n";
        ret.first = NIL;
        return ret;
      }
    }
    ret.first = MONITOR;
  }
  else if (comm == "unmonitor" || comm == "um") {
    if (v[1] == "") {
      printf("USAGE: unmonitor [r0-r15/f0-f15/m{{address}}]\n");
    }
    else if (!v[1].compare(0, 1, "R") || !v[1].compare(0, 1, "r")) {
      int no = std::stoi(v[1].substr(1));
      regs_to_monitor.erase(no);
    }
    else if (!v[1].compare(0, 1, "F") || !v[1].compare(0, 1, "f")) {
      int no = std::stoi(v[1].substr(1));
      fregs_to_monitor.erase(no);
    }
    else if (!v[1].compare(0, 1, "M") || !v[1].compare(0, 1, "m")) {
      long no = std::stol(v[1].substr(1));
      address_to_monitor.erase(no);
    }
    ret.first = UNMONITOR;  // unmonitor
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
        else if (!v[i].compare(0, 1, "M") || !v[i].compare(0, 1, "m")) {
          if (!v[i].compare(1, 1, "I") || !v[i].compare(1, 1, "i")) {  // memory (int)
            long no = std::stol(v[i].substr(2));
            intaddress_to_show.emplace(no);
          }
          else if (!v[i].compare(1, 1, "F") || !v[i].compare(1, 1, "f")) {  // memory (flt)
            long no = std::stol(v[i].substr(2));
            fltaddress_to_show.emplace(no);
          }
          else {
            long no = std::stol(v[i].substr(1));
            address_to_show.emplace(no);
          }
        }
        else {printf("USAGE: print [r0-r15/f0-f15]\n"); ret.first = NIL;}
      }
    }
    ret.first = PRINT;
  }
  else if (comm == "fast" || comm == "f") {   // fast
    ret.first = FAST;
    if (v.size() == 2) {
      long count = std::stol(v[1]);
      ret.second= count;
    }
    else if (v.size() > 2) {printf("USAGE: fast {{max_count_of_silent_execution}}\n"); ret.first = NIL;}
  }
  else if (comm == "at" || comm == "a") {     // at
    std::cout << "pc: " << pc << std::endl;
    std::cout << "line: " << ninsts.at(pc) << std::endl;
    std::cout << "executed instructions: " << total_executed << std::endl;
    ret.first = NIL;
  }
  else if (comm == "clear" || comm == "c") {  // clear
    if (v.size() == 1) {
      regs_to_show.clear();
      fregs_to_show.clear();
      address_to_show.clear();
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
        else if (!v[i].compare(0, 1, "M") || !v[i].compare(0, 1, "m")) {
          if (!v[i].compare(1, 1, "I") || !v[i].compare(1, 1, "i")) {  // memory (int)
            long no = std::stol(v[i].substr(2));
            intaddress_to_show.erase(no);
          }
          else if (!v[i].compare(1, 1, "F") || !v[i].compare(1, 1, "f")) {  // memory (flt)
            long no = std::stol(v[i].substr(2));
            fltaddress_to_show.erase(no);
          }
          else {
            long no = std::stol(v[i].substr(1));
            address_to_show.erase(no);
          }
        }
        else {printf("USAGE: clear [r0-r15/f0-f15]\n"); ret.first = NIL;}
      }
    }
    ret.first = CLEAR;
  }
  else if (comm == "help" || comm == "h") {   // help
    show_help();
    ret.first = HELP;
  }
  else if (comm == "run" || comm == "r") {    // run
    ret.first = RUN;
    if (v.size() == 2) {
      long count = std::stol(v[1]);
      ret.second = count;
    }
    else if (v.size() > 2) {printf("USAGE: run {{max_count_of_verboosed_instruction}}\n"); ret.first = NIL;}
  }
  else {
    std::cerr << "Unknown command: " << s << std::endl;
    ret.first = ERR;
  }

  return ret;
}

void test(void)
{
  while (1) {
    if (exec_inst() == NOP) break;
  }
}

int main(int argc, char **argv)
{
  if (argc != 7) {
    printf("USAGE: %s {{binary}} {{labels}} {{insts}} {{ofs}} {{test_flag}} {{input_file}}\n", argv[0]);
    exit(1);
  }

  if (!strcmp(argv[5], "1") || !strcmp(argv[5], "test") || !strcmp(argv[5], "true")) {
    test_flag = 1;
  }

  std::cout << "------------------------------\n";
  puts("Running simulator...");

  init();

  init_inst(argv[1]);
  init_labels(argv[2]);
  init_ninsts(argv[3]);
  init_ofs(argv[4]);
  init_fin(argv[6]);

  show_help();
  putchar('\n');

  try {
    if (test_flag) test();
    else {
      std::string s;
      while (1) {
        printf("%s", PROMPT);
        if (!std::getline(std::cin, s)) break;
        std::pair<Comm,long> res = analyze_commands(s);
        Comm comm = res.first;
        long count = res.second;

        if (comm == NIL) continue;
        else if (comm == QUIT) break;
        else if (comm == ERR) continue;
        else if (comm == STEP) exec_inst();
        else if (comm == RUN) {
          if (count == 0) {
            while (1) {
              if (monitor()) break;
              Comm ret = exec_inst();
              if (ret == NOP) break;
              else if (ret == BREAK) break;
              if (breakpoint < 0) continue;
              else if (pc == (unsigned)breakpoint) break;
            }
          }
          else if (count > 0) {
            while (count-- > 0) {
              if (monitor()) break;
              Comm ret = exec_inst();
              if (ret == NOP) break;
              else if (ret == BREAK) break;
              if (breakpoint < 0) continue;
              else if (pc == (unsigned)breakpoint) break;
            }
          }
        }
        else if (comm == FAST) {
          if (count == 0) exec_inst_silent();
          else if (count > 0) {
            if (count >= 5) {
              if (monitor()) break;
              Comm ret = exec_inst_silent(count-5);
              if (ret == NOP) break;
              else if (ret == BREAK) break;
              else if (ret == MONITOR) break;
              for (int i=0; i<5; i++) {
                ret = exec_inst();
                if (ret == NOP) break;
                else if (ret == BREAK) break;
                else if (ret == MONITOR) break;
              }
            }
          }
        }
        else if (comm == UNBREAK) {breakpoint = -1; continue;}
        else ;
        print_regs();
        reset_bold();
      }
    }
  }
  catch (const std::out_of_range& e) {  // memory range (most likely)
    std::cerr << e.what() << std::endl;
    std::string s = FormatWithCommas(total_executed);
    std::cerr << "died at " << s << "th instruction of pc: " << pc << ", line: " << ninsts.at(pc) << "\n";
  }
  catch (const std::runtime_error& e) {  // runtime error
    std::cerr << e.what() << std::endl;
    std::string s = FormatWithCommas(total_executed);
    std::cerr << "died at " << s << "th instruction of pc: " << pc << ", line: " << ninsts.at(pc) << "\n";
  }

  puts("\nsimulator terminated");
  std::cout <<"total executed instructions: " << FormatWithCommas(total_executed) << std::endl;
  //printf("max sp(r29): %ld, max hp(r30): %ld\n", r29_max, r30_max);

  free(inst_reg);
  ofs.flush();
  ofs.close();

  return 0;
}
