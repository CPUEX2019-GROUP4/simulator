#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>

#include "stringutils.hh"
#include "assembleutils.hh"

int main(int argc, char **argv)
{
  if (argc != 3 && argc != 2) {
    print_help(argv[0]);
    exit(1);
  }

  std::ifstream ifs(argv[1]);
  std::ofstream ofs;

  if (argc >= 3) ofs.open(argv[2], std::ios::out|std::ios::binary|std::ios::trunc);
  else {
    printf("output file is to be saved to 'a.out'\n");
    ofs.open("a.out", std::ios::out|std::ios::binary|std::ios::trunc);
  }

  if (ifs.fail()) {std::cerr << "failed to open " << argv[1] << "\n"; exit(1);}
  if (ofs.fail()) {std::cerr << "failed to open " << argv[2] << "\n"; exit(1);}

  std::cout << "------------------------------\n";

  std::string inst;

  while (!ifs.eof()) {
    getline(ifs, inst);
    if (!inst.compare("")) break;

    std::vector<std::string> v;
    inst = split(inst, "#", false)[0];
    v = split(inst, " ");
    if (v.empty()) continue;

    try {
      uint32_t ret = assemble(v);
      ofs.write((char*)(&ret), 4);
    }
    catch (const int& e) {
      std::cerr << "imm was " << e << " at the instruction '" << inst << "'\n";
      exit(1);
    }
  }

  ifs.close();
  ofs.close();

  printf("Assembled successfully.\n");
  printf("%s ==> %s\n", argv[1], argv[2]);

  return 0;
}
