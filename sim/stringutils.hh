#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <locale>

/**--- std::string をdelimiterで分割してstd::vectorで返す ---*/
std::vector<std::string> split(std::string s, std::string delimiter,
                               bool shrink = true);

/**--- 文字列中の整数にコンマを挿入する ---*/
template<class T> std::string FormatWithCommas(T value)
{
  std::stringstream ss;
  ss.imbue(std::locale(""));
  ss << std::fixed << value;
  return ss.str();
}
