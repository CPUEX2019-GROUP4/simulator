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

std::vector<std::pair<int,int>> line_inst_list;
std::vector<std::pair<std::string,int>> label_inst_list;

/** line_inst_list, label_inst_list をセット */
void gather(std::string path)
{
  std::ifstream ifs(path);
  if (ifs.fail()) {std::cerr << "File " << path << "cannot be opened. Abort\n"; exit(1);}

  std::string s;
  std::vector<std::string> v;

  int l = 0;  // line number
  int i = 0;  // inst number

  while (1) {
    l++;
    if (!std::getline(ifs, s)) break;
    s = split(v, "#")[0]; // 行の途中のコメントは削除
    v = split(s, " ");
    if (v.empty()) continue;
    //if (v[0][0] == '#') continue;
    s = v[0];
    v = split(s, ":");
    if (v.size() > 1) label_inst_list.push_back(std::make_pair(v[0], i));
    else line_inst_list.push_back(std::make_pair(l, i++));
  }
  ifs.close();
}

void output_inst(std::vector<std::pair<int,int>> line_inst_list, std::string path = "inst.txt")
{
  std::string s = "";
  std::ofstream ofs(path);
  if (ofs.fail()) {std::cerr << "File " << path << "cannot be opened. Abort\n"; exit(1);}
  for (auto it : line_inst_list) {
    s += *it[1] + " " + *it[0] + "\n";
  }
  ofs << s;
}

void output_labels(std::vector<std::pair<std::string,int>> label_inst_list, std::string path = "label.txt")
{
  std::string s = "";
  std::ofstream ofs(path);
  if (ofs.fail()) {std::cerr << "File " << path << "cannot be opened. Abort\n"; exit(1);}
  for (auto it : label_inst_list) {
    s += *it[0] + " " + *it[1] + "\n";
  }
  ofs << s;
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

int main(int argc, char **argv)
{
  gather("foo.s");
  output_inst("ins.txt");
  output_labels("lab.txt");

  return 0;
}
