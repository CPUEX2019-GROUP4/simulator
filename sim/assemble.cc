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

  std::string ofsname = (argc >= 3)? argv[2] : "a.out";
  ofs.open(ofsname, std::ios::out|std::ios::binary|std::ios::trunc);
  std::cout << "output file is to be saved to " <<  ofsname << ".\n";

  if (ifs.fail()) {std::cerr << "failed to open " << argv[1] << "\n"; exit(1);}
  if (ofs.fail()) {std::cerr << "failed to open " << argv[2] << "\n"; exit(1);}

  std::cout << "------------------------------\n";

  std::string inst;

  long l = 0;   // line number

  while (!ifs.eof()) {
    l++;
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
      std::cerr << "see the " "\033[1m" << l << "th" "\033[m";
      std::cerr << " line of "<< argv[1] << ".\n";
      exit(1);
    }
    catch (const std::invalid_argument& e) {
      ofs.close();
      std::cout << e.what() << std::endl;
      std::cerr << "\033[1m" "invalid argument " "\033[m";
      std::cerr << "at the instruction '" "\033[1m" << inst << "\033[m" "'\n";
      std::cerr << "see the " "\033[1m" << l << "th" "\033[m";
      std::cerr << " line of "<< argv[1] << ".\n";
      exit(1);
    }
    catch (const std::string& e) {
      std::cerr << e << "\033[m\n";
      std::cerr << "see the " "\033[1m" << l << "th" "\033[m";
      std::cerr << " line of "<< argv[1] << ".\n";
      exit(1);
    }
  }

  ifs.close();
  ofs.close();

  printf("Assembled successfully.\n");
  printf("%s ==> %s\n", argv[1], argv[2]);

  return 0;
}
