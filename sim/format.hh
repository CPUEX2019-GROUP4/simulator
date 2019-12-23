#pragma once
#include <string>

typedef enum {R=1, F=2, I=4, O=8, N=16} oprand_t;  // O: others, N: nil

//extern std::pair<std::string, oprand_t[3]>table;

oprand_t type(std::string s);
void format_check(std::string infile);

void init_table();
