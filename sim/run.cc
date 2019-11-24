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
#define LEN_INSTRUCTION 100000
#define N_REG 32
#define SIZE_MEM (2<<20)

#define $rd (int_reg[get_rd(inst)])
#define $ra (int_reg[get_ra(inst)])
#define $rb (int_reg[get_rb(inst)])
#define $fd (float_reg[get_rd(inst)])
#define $fa (float_reg[get_ra(inst)])
#define $fb (float_reg[get_rb(inst)])

// registers
uint32_t *inst_reg;           // instruction register
uint32_t pc = 0;                   // program counter
int32_t int_reg[N_REG];       // int
float   float_reg[N_REG];     // float
std::array<char, SIZE_MEM> mem;           // memory
int32_t fcond_reg;

// Meta variables
uint32_t total_inst = 0;
std::unordered_map<int,int> ninsts;  // 命令番号に対するソースコード行番号のmap
std::unordered_map<int,int> rev_ninsts;  // ↑の逆
std::unordered_map<std::string,int> labels;  // ラベルに対するソースコード行番号のmap
std::ofstream ofs;                // OUT 命令の出力ファイル
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

template<class T>
std::string FormatWithCommas(T value)
{
  std::stringstream ss;
  ss.imbue(std::locale(""));
  ss << std::fixed << value;
  return ss.str();
}

inline void init_ofs(char *path)
{
  ofs.open(path, std::ios::out | std::ios::trunc); // append はつけない
  if (ofs.fail()) {std::cerr << "(ofs)File '" << path << "' could not be opened\n"; exit(1);}
}

inline void init_fin(char *path)
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
//int32_t get_shift_signed(uint32_t inst) {return (int16_t)(inst & 0x3ff);}
uint32_t get_func(uint32_t inst) {return (inst >> 0) & 0x3f;}
uint16_t get_imm(uint32_t inst) {return (inst >> 0) & 0xffff;}
int16_t get_imm_signed(uint32_t inst) {return (inst & 0x7fff) - (inst & 0x8000);}
//int16_t get_imm_signed(uint32_t inst) {return (int16_t)(inst & 0x7fff);}
uint32_t get_addr(uint32_t inst) {return (inst >> 0) & 0x3ffffff;}

int main(int argc, char **argv)
{
  if (argc != 7) {
    printf("USAGE: %s {{binary}} {{labels}} {{insts}} {{ofs}} {{test_flag}} {{input_file}}\n", argv[0]);
    exit(1);
  }

  std::cout << "------------------------------\n";
  puts("Running simulator...");

  init_inst(argv[1]);
  init_labels(argv[2]);
  init_ninsts(argv[3]);
  //init_ofs(argv[4]);
  ofs.open(argv[4], std::ios::out | std::ios::trunc); // append はつけない
  if (ofs.fail()) {std::cerr << "(ofs)File '" << argv[4] << "' could not be opened\n"; exit(1);}
  //init_fin(argv[6]);
  fin = fopen(argv[6], "r");
  if (!fin) {std::cerr << "(fin)File '" << argv[6] << "' could not be opened\n"; exit(1);}

  puts("run only mode!!");

  try {
    while (1) {
      if (!(0 <= pc && pc < total_inst)) {
        printf("pc out of index. Abort. %d of %d\n", pc, total_inst);
        exit(1);
      }
      uint32_t inst = inst_reg[pc];

      total_executed++;

      //if (int_reg[29] > r29_max) r29_max = int_reg[29];
      //if (int_reg[30] > r30_max) r30_max = int_reg[30];

      if (inst == 0) {printf("nop\n"); break;}   // nop

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
              {
                uint32_t s_, e_;
                b.f = $fa;
                s_ = b.ui32 & 0x80000000;
                e_ = b.ui32 & 0x7f800000;
                e_ = (e_ >> 1) + (64 << 23);
                b.ui32 = s_ | e_;
                $fd = b.f;
              }
            case 0x38:      // finv_init
              {
                uint32_t s_, e_;
                b.f = $fa;
                s_ = b.ui32 & 0x80000000;
                e_ = b.ui32 & 0x7f800000;
                e_ = (127 << 23) - e_;
                e_ += (127 << 23);
                b.ui32 = s_ | e_;
                $fd = b.f;
              }
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
          memcpy((char*)(&($rd)), &mem.at($ra + get_imm_signed(inst)), 4);
          pc++; break;
        case 0x2b:      // sw
          memcpy(&mem.at($ra + get_imm_signed(inst)), (char*)(&($rd)), 4);
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
          memcpy((char*)(&($fd)), &mem.at($ra + get_imm_signed(inst)), 4);
          pc++;
          break;
        case 0x38:      // swcZ
          memcpy(&mem.at($ra + get_imm_signed(inst)), (char*)(&($fd)), 4);
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
