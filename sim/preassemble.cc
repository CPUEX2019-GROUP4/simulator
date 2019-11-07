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

std::vector<std::string> split(std::string s, std::string delimiter, bool shrink=true);

std::unordered_map<int,int> line_inst_list;
std::unordered_map<int,int> inst_line_list;
std::unordered_map<std::string,int> label_inst_list;

/** line_inst_list, label_inst_list をセット */
void gather(std::string path)
{
  std::ifstream ifs(path);
  if (ifs.fail()) {std::cerr << "File " << path << " cannot be opened. Abort\n"; exit(1);}

  std::string s;
  std::vector<std::string> v;

  int l = 0;  // line number
  int i = 0;  // inst number

  while (1) {
    l++;
    if (!std::getline(ifs, s)) break;
    std::string ss = split(s, "#", false)[0]; // 行の途中のコメントは削除
    v = split(ss, " ");
    if (v.empty()) continue;
    //if (!v[1].compare("")) {std::cout << "Empty (" << l << "): " << s << "\n"; continue;}
    s = v[0];
    v = split(s + "hoge\n", ":", false);
    if (v.size() > 1) label_inst_list.emplace(std::make_pair(v[0], i));
    else {
      line_inst_list.emplace(std::make_pair(l, i));
      inst_line_list.emplace(std::make_pair(i, l));
      i++;
    }
  }
  ifs.close();
}

void subst_labels(std::string infile, std::string outfile)
{
  std::ifstream ifs(infile, std::ios::in);
  if (ifs.fail()) {std::cerr << "File " << infile << " cannot be opened. Abort\n"; exit(1);}
  std::ofstream ofs(outfile, std::ios::out | std::ios::trunc);
  if (ofs.fail()) {std::cerr << "File " << outfile << " cannot be opened. Abort\n"; exit(1);}

  std::string s;
  std::vector<std::string> v;

  int l = 0;  // line number
  while (1) {
    l++;
    if (!std::getline(ifs, s)) break;
    std::string ss = split(s, "#", false)[0]; // 行の途中のコメントは削除
    v = split(ss, " ");
    if (v.empty()) {ofs << s << "\n"; continue;} // comment
    std::vector<std::string> vv = split(s + "foo", ":", false);
    if (vv.size() > 1) {ofs << "###__" << s << "__\n"; continue;} // label declaration
    if (!s.compare("")) {std::cout << "nothing\n"; continue;}

    std::string opcode = v[0];
    std::string str;

    if (!opcode.compare("jal") || !opcode.compare("j")) ofs << opcode << " " << label_inst_list.at(v[1]) << "\t\t\t# " << v[1] << "\n";
    else if (!opcode.compare("bc1f") || !opcode.compare("bc1t")) {
      int n = label_inst_list.at(v[1]) - line_inst_list[l] - 1;
      ofs << opcode << " " << std::to_string(n) << "\t\t\t# " << v[1] << "\n";
    }
    else if (!opcode.compare("bne") || !opcode.compare("beq")) {
      int n = label_inst_list.at(v[3]) - line_inst_list[l] - 1;
      ofs << opcode << " " << v[1] << " " << v[2] << " " << std::to_string(n) << "\t\t\t# " << v[3] << "\n";
    }
    else if (!opcode.compare("mv")) ofs << "or " << v[1] << " r0 " << v[2] <<"\n";
    else if (!opcode.compare("subi")) ofs << "addi " << v[1] << " " << v[2]  << " -" << v[3] <<"\n";
    else if (!opcode.compare("lui")) {
      std::vector<std::string> vv = split(v[2], "ha16(", false);
      if (vv.size() == 1) ofs << s << std::endl;  // line without ha16
      else if (vv.size() == 2) {
        std::string ss = split(vv[1], ")")[0];
        uint32_t n = (uint32_t)label_inst_list.at(ss) & 0xffff0000;
        ofs << opcode << " " << v[1] << " " << std::to_string(n) << "\t\t\t# " << ss << "\n";
      }
      else if (vv.size() > 2) {
        std::cerr << "Too many halo16 in a line(" << l << ". Abort.\n"; exit(1);  // unexpected
      }
    }
    else if (!opcode.compare("ori")) {
      std::vector<std::string> vv = split(v[3], "lo16(", false);
      if (vv.size() == 1) ofs << s << std::endl;  // line without lo16
      else if (vv.size() == 2) {
        std::string ss = split(vv[1], ")")[0];
        uint32_t n = (uint32_t)label_inst_list.at(ss) & 0xffff;
        ofs << opcode << " " << v[1] << " " << v[2] << " " << std::to_string(n) << "\t\t\t# " << ss << "\n";
      }
      else if (vv.size() > 2) {
        std::cerr << "Too many halo16 in a line(" << l << ". Abort.\n"; exit(1);
      }
    }
    else ofs << s << std::endl;
  }
  ifs.close();
  ofs.close();
}

void output_inst(std::unordered_map<int,int> line_inst_list, std::string path = "inst.txt")
{
  std::string s = "";
  std::ofstream ofs(path);
  if (ofs.fail()) {std::cerr << "File " << path << " cannot be opened. Abort\n"; exit(1);}
  for (auto it : line_inst_list) {
    s += std::to_string(it.second);
    s.append(" ");
    s += std::to_string(it.first);
    s.append("\n");
  }
  ofs << s;
}

void output_labels(std::unordered_map<std::string,int> label_inst_list, std::string path = "label.txt")
{
  std::string s = "";
  std::ofstream ofs(path);
  if (ofs.fail()) {std::cerr << "File " << path << " cannot be opened. Abort\n"; exit(1);}
  for (auto it : label_inst_list) {
    s += it.first;
    s.append(" ");
    s += std::to_string(it.second);
    s.append("\n");
  }
  ofs << s;
}

/**--- std::string をdelimiterで分割してstd::vectorで返す ---*/
std::vector<std::string> split(std::string s, std::string delimiter, bool shrink)
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
      if (tmp == std::string::npos) {v.push_back(s.substr(pos, tmp-pos)); break;}
      v.push_back(s.substr(pos, tmp-pos));
      pos = tmp + delimiter.length();
    }
  }
  return v;
}

void test()
{
  std::string s = "    #aaa";
    s = split(s, "#", false)[0]; // 行の途中のコメントは削除
    std::vector<std::string> v = split(s, " ");
    if (v.empty()) {std::cout << "empty\n";}
    std::vector<std::string> vv = split(s + "foo", ":", false);
    if (vv.size() > 1) {std::cout << "vv " << vv.size() << std::endl;}
    if (!s.compare("")) std::cout << "\"\"" << std::endl;
  for (auto it : v) {
    std::cout << it << std::endl;
  }
  //std::cout << s << std::endl;
}

int main(int argc, char **argv)
{
  std::string in = "foo.s";
  std::string out = "piyo.s";

  if (argc >= 2) in = argv[1];
  if (argc == 3) out = argv[2];
  if (argc > 3) {std::cerr << "USAGE: " << argv[0] << "{{in_file}} {{preassembled_file}}\n"; exit(1);}

  std::cout << in << " ==> " << out << std::endl;
  gather(in);
  output_inst(line_inst_list);
  output_labels(label_inst_list);
  subst_labels(in, out);
  //test();

  return 0;
}
