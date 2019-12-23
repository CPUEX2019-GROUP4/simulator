#include "format.hh"
#include <algorithm>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "stringutils.hh"

std::unordered_map<std::string, oprand_t[3]>table;

void init();

oprand_t type(std::string s)
{
  if (!s.compare("")) return N;
  else if (std::all_of(s.cbegin(), s.cend()-1, isdigit)) {
    return I;
  }
  else if (s[0] == '-' && std::all_of(s.cbegin()+1, s.cend()-1, isdigit)) {
    return I;
  }
  else if (s[0] == 'r' && std::isdigit(static_cast<unsigned char>(s[1]))) {
    return R;
  }
  else if (s[0] == 'f' && std::isdigit(static_cast<unsigned char>(s[1]))) {
    return F;
  }
  else return O;  // others
}

void format_check(std::string infile)
{
  std::ifstream ifs(infile, std::ios::in);
  if (ifs.fail()) {std::cerr << "File " << infile << " cannot be opened. Abort\n"; exit(1);}

  std::string s;
  std::vector<std::string> v;

  int l = 0;  // line number

  init_table();

  while (1) {
    l++;
    if (!std::getline(ifs, s)) break;
    std::string ss = split(s, "#", false)[0]; // 行の途中のコメントは削除
    v = split(ss, " ");
    if (v.empty()) continue; // comment line or empty line
    std::vector<std::string> vv = split(s + "foo", ":", false);
    if (vv.size() > 1) continue;  // label declaration
    if (!s.compare("")) {std::cout << "nothing\n"; continue;}

    if (v.size() > 4) {
      std::cerr << "too many operands at line: " << l << ".\n";
      std::cerr << "the instruction is: " << s << ".\n";
      exit(1);
    }

    std::string opcode = v[0];  // opcode

    for (int i=1; i<(int)v.size(); i++) {
      v[i] = split(v[i], "\t")[0];      // tabs are not separator b/w operands,
                                        // so just take the first element
      //std::cout << "i: " << i << "type(v[i]) = " << type(v[i]) << "table: " << table[opcode][i-1] << "\n";
      //std::cout << "v[i]=" << v[i] << ".\n";
      if (type(v[i]) != table[opcode][i-1]) { //XXX: type `N` is not used
        std::cerr << "Found invalid operand type at line " << l << ".\n";
        std::cerr << "the instruction is: " << s << ".\n";
        exit(1);
      }
    }
  }
  std::cout << "operand type check ok.\n";
  ifs.close();
  return;
}

void init_table()
{
#define T(str) table[#str]
#define ASSIGN(e,a,b,c) do {T(e)[0] = a; T(e)[1] = b; T(e)[2] = c;} while(0)
  // general register
  ASSIGN(nop, N, N, N);
  ASSIGN(add, R, R, R);
  ASSIGN(sub, R, R, R);
  ASSIGN(div2, R, R, N);
  ASSIGN(div10, R, R, N);
  ASSIGN(addi, R, R, I);
  ASSIGN(lw, R, R, I);
  ASSIGN(sw, R, R, I);
  ASSIGN(lui, R, I, N);
  ASSIGN(or, R, R, R);
  ASSIGN(ori, R, R, I);
  ASSIGN(slt, R, R, R);
  ASSIGN(slti, R, R, I);
  ASSIGN(sllv, R, R, R);
  ASSIGN(sll, R, R, I);
  ASSIGN(beq, R, R, I);
  ASSIGN(bne, R, R, I);
  ASSIGN(j, I, N, N);
  ASSIGN(jr, R, N, N);
  ASSIGN(jal, I, N, N);
  ASSIGN(jalr, R, N, N);
  // FP
  ASSIGN(ftoi, R, F, N);
  ASSIGN(itof, F, R, N);
  ASSIGN(fneg, F, F, N);
  ASSIGN(fadd, F, F, F);
  ASSIGN(fsub, F, F, F);
  ASSIGN(fmul, F, F, F);
  ASSIGN(fdiv, F, F, F);
  ASSIGN(lwcZ, F, R, I);
  ASSIGN(swcZ, F, R, I);
  ASSIGN(fclt, F, F, N);
  ASSIGN(fcz, F, N, N);
  ASSIGN(bc1t, I, N, N);
  ASSIGN(bc1f, I, N, N);
  ASSIGN(flui, F, I, N);
  ASSIGN(fori, F, F, I);
  ASSIGN(fmv, F, F, N);
  ASSIGN(sqrt_init, F, F, N);
  ASSIGN(finv_init, F, F, N);
  // in/out
  ASSIGN(out, R, I, N);
  //ASSIGN(inint, R, N, N);
  //ASSIGN(inflt, F, N, N);
  ASSIGN(inint, R, R, N); // XXX
  ASSIGN(inflt, F, R, N); // XXX

#undef ASSIGN
#undef T
}
