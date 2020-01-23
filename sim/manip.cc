#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <algorithm>

#include "stringutils.hh"

#define POSTFIX ".gen"

std::unordered_map<std::string,int> t; // table
std::string SOURCE;
std::vector<std::string> TARGETS;
std::string LTOKEN;
std::string RTOKEN;
std::string TOKEN;
int NINDENT;

void print_usage(char *program_name)
{
  std::cout << "USAGE: " << program_name << " {{config_file}}\n";
}

void set_table(char *filename)
{
  std::ifstream ifs(filename);
  if (ifs.fail()) {std::cerr << "File " << filename << " cannot be opend. Abort\n"; exit(1);}

  std::string s;
  std::vector<std::string> v;
  while (1) {
    if (!std::getline(ifs, s)) break;
    if (!s.compare("")) continue;  // 空行
    std::string ss = split(s, "#", false)[0]; // 行の途中のコメントを削除
    v = split(ss, " ");
    if (v.empty()) continue;      // コメントを除くと空行
    if (!v[0].compare("EOF")) break;  // end of settings
    if (v.size() < 3 || v[1].compare("=")) {
      std::cout << "Warning! This line was ignored: " << s << std::endl;
      continue;
    }

    if (!v[0].compare("SOURCE")) SOURCE = v[2];
    else if (!v[0].compare("LTOKEN")) LTOKEN = v[2];
    else if (!v[0].compare("RTOKEN")) RTOKEN = v[2];
    else if (!v[0].compare("TOKEN")) TOKEN = v[2];
    else if (!v[0].compare("NINDENT")) NINDENT = std::stoi(v[2]);
    else if (!v[0].compare("TARGETS")) {
      for (int i=2; i<(int)v.size(); i++) TARGETS.push_back(v[i]);
    }
  }
}

void scan_source(std::ifstream& ifs) {

  std::string s;
  std::vector<std::string> v;
  while (1) {
    // XXX: Only accept L/RTOKEN with 0 or more leading whitespaces
    //      anything else such as tabstops are not allowed
    if (!std::getline(ifs, s)) break;
    if (!s.compare("")) continue;     // 空行
    std::string ss = split(s, " ")[0];            // 行頭のスペース除去
    v = split("foo"+ss, LTOKEN);
    if (!split(v[0], " ")[0].compare("foo")) {  // found the LTOKEN
      t[v[1]] = ifs.tellg();
    }
  }
}

int count_leading_whitespaces(std::string s) {
  int ret = 0;
  while (s[ret] == ' ') ret++;
  return ret;
}

int count_depth(std::string s) {
  int ret = 0;
  ret += std::count(s.begin(), s.end(), '(');
  ret -= std::count(s.begin(), s.end(), ')');
  ret += std::count(s.begin(), s.end(), '{');
  ret -= std::count(s.begin(), s.end(), '}');
  ret += std::count(s.begin(), s.end(), '[');
  ret -= std::count(s.begin(), s.end(), ']');
  return ret;
}

void paste_to_target(std::ifstream& fs, std::ifstream& ifs, std::ofstream& ofs) {
  std::string s;
  std::vector<std::string> v;
  long l = 0;
  while (1) {
    l++;
    if (!std::getline(fs, s)) break;
    ofs << s << std::endl;
    if (!s.compare("")) continue;     // 空行
    std::string ss = split(s, " ")[0];            // 行頭のスペース除去
    v = split("foo"+ss, TOKEN);
    if (!split(v[0], " ")[0].compare("foo")) {  // found the TOKEN
      // count the leading whitespaces
      int count = count_leading_whitespaces(s);
      if (v.size() < 2) {
        std::cerr << "Warning! Empty token name found at line " << l << ".\n";
        continue;
      }
      //std::cout << v[1] << "(ws): " << count << std::endl;

      int level = 0;
      std::vector<std::string> vv;
      vv = split(v[1], "@");
      if (vv.size() > 1) {
        try {
          level = std::stoi(split(vv[1], " ")[0]);
        }
        catch (const std::exception& e) {
          std::cerr << e.what() << std::endl;
          std::cerr << "see " << l << "th line.\n";
        }
      }
      int level2 = level * 2;

      // start copying from ifs
      ifs.clear();          // EOFフラグをクリアしないとseekg()できない
      ifs.seekg(t[vv[0]]);

      int flag = 1;
      int depth2 = 0;
      while (flag) {
        if (!std::getline(ifs, s)) flag = 0;
        ss = split(s, " ")[0];            // 行頭のスペース除去
        v = split("foo"+ss, RTOKEN);
        if (!split(v[0], " ")[0].compare("foo")) {  // found the RTOKEN
          flag = 0;
        }
        if (!s.compare("")) {     // 空行
          ofs << "\n";
          continue;
        }
        depth2 += 2 * count_depth(s);
        if (count_leading_whitespaces(s) - depth2 > level2) continue;  // 大きいlevel
        for (int i=0; i<count; i++) ofs << " ";
        ofs << s << std::endl;
      }
    }
  }
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    print_usage(argv[0]);
    exit(1);
  }

  set_table(argv[1]);

  std::cout << "TARGETS: ";
  for (auto it : TARGETS) std::cout << it << " ";
  std::cout << std::endl;

  //std::cout << "TOKEN: " << TOKEN << std::endl;

  std::ifstream ifs(SOURCE);
  if (ifs.fail()) {std::cerr << "File " << SOURCE << " cannot be opend. Abort\n"; exit(1);}

  scan_source(ifs);

  for (auto it : TARGETS) {
    std::ifstream fs(it);
    std::ofstream ofs(it+POSTFIX);
    if (fs.fail()) {std::cerr << "File " << it << " cannot be opend. Abort\n"; exit(1);}
    if (ofs.fail()) {std::cerr << "File " << it+POSTFIX << " cannot be opend. Abort\n"; exit(1);}

    std::cout << "Manip on " << it+POSTFIX << " ...\n";
    paste_to_target(fs, ifs, ofs);
    fs.close();
  }

  std::cout << "All done.\n";

  ifs.close();

  return 0;
}
