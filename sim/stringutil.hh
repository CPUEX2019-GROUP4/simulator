#ifndef __STRINGUTIL_HH__
#define __STRINGUTIL_HH__

/**--- std::string をdelimiterで分割してstd::vectorで返す ---*/
std::vector<std::string> split(std::string s, std::string delimiter, bool shrink=true);

#endif
