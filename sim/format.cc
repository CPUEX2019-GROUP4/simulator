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

    //for (int i=1; i<(int)v.size(); i++) {
    for (int i=1; i<=3; i++) {
      if (table[opcode][i-1] == N) {
        if ((int)v.size() > i) {
          std::cerr << "Too many operands at line: " << l << ".\n";
          std::cerr << "the instruction is: " << s << ".\n";
          //pass = 0;
          exit(1);
        }
        break;
      }
      else {  // expecting some operand
        if ((int)v.size() <= i) {
          if (table[opcode][i-1] == O) break; // XXX: forgive
          std::cerr << "Too few operands at line: " << l << ".\n";
          std::cerr << "the instruction is: " << s << ".\n";
          //pass = 0;
          exit(1);
        }
      }
      v[i] = split(v[i], "\t")[0];      // tabs are not separator b/w operands,
                                        // so just take the first element
      if (type(v[i]) != table[opcode][i-1]) { //XXX: type `N` is not used
        if (table[opcode][i-1] == O) break; // XXX: forgive
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


void format_check_validate(std::string infile)
{

  std::unordered_map<std::string, std::vector<std::string>> results;

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
    int pass = 1;

    //for (int i=1; i<(int)v.size(); i++) {
    for (int i=1; i<=3; i++) {
      if (table[opcode][i-1] == N) {
        if ((int)v.size() > i) {
          std::cerr << "Too many operands at line: " << l << ".\n";
          std::cerr << "the instruction is: " << s << ".\n";
          pass = 0;
        }
        break;
      }
      else {  // expecting some operand
        if ((int)v.size() <= i) {
          if (table[opcode][i-1] == O) break; // XXX: forgive
          std::cerr << "Too few operands at line: " << l << ".\n";
          std::cerr << "the instruction is: " << s << ".\n";
          pass = 0;
          break;
        }
      }

      //std::cout << "checking: " << s << ", i=" << i << ".\n";

      v[i] = split(v[i], "\t")[0];      // tabs are not separator b/w operands,
                                        // so just take the first element
      if (type(v[i]) != table[opcode][i-1]) { //XXX: type `N` is not used
        if (table[opcode][i-1] == O) break; // XXX: forgive
        //std::cerr << "\033[1m" << "invalid: " << s <<  "\033[m" ".\n";
        //exit(1);
        pass = 0;
        break;
      }
      /*
      else {
        std::cout << "v[i]: " << v[i] << " type(v[i])=" << type(v[i]) << " talble: " << table[opcode][i-1] << ".\n";
      }
      */
    }
    if (pass) {
      std::cout << "pushing: " << s << ".\n";
      results[opcode].push_back(s);
    }
  }

  std::cout << "\033[1m" "SUMMARY" "\033[m" "\n";

  for (auto it : results) {
    std::cout << "\033[1m" << it.first << "\033[m" << "\n";
      for (auto itt : it.second) std::cout << itt << std::endl;
  }
  //std::cout << "operand type check ok.\n";
  ifs.close();
  return;
}

void init_table()
{
#define T(str) table[#str]
#define ASSIGN(e,a,b,c) do {T(e)[0] = a; T(e)[1] = b; T(e)[2] = c;} while(0)
  // 2nd
  ASSIGN(lwab, R, R, R);
  ASSIGN(swab, R, R, R);
  ASSIGN(flw, F, R, I);
  ASSIGN(fsw, F, R, I);
  ASSIGN(flwab, F, R, R);
  ASSIGN(fswab, F, R, R);
  ASSIGN(blt, R, R, I);
  ASSIGN(seq, R, R, R);
  ASSIGN(fabs, F, F, N);
  ASSIGN(feq, F, F, N);
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
  ASSIGN(lwcZ, F, R, I);  // XXX: temporary
  ASSIGN(swcZ, F, R, I);  // XXX: temporary
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
  //ASSIGN(inint, R, R, N); // XXX
  //ASSIGN(inflt, F, R, N); // XXX
  ASSIGN(inint, R, O, N); // XXX
  ASSIGN(inflt, F, O, N); // XXX

#undef ASSIGN
#undef T
}
