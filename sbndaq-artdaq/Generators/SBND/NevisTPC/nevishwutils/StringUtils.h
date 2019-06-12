#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H 1

#include <string>

namespace nevistpc {

char toupper(char i);
char tolower(char i);

std::string to_upper(std::string const & value);
std::string to_lower(std::string const & value);

std::string filterString(std::string const & value,
                         std::string const & pattern);

std::string getCmdOption(char ** begin, char ** end,  std::string const& option);
bool cmdOptionExists(char** begin, char** end,  std::string const& option);

std::string removeExtension(const std::string & filename);
std::string getTimestampAsString(std::string format = "%d%m%Y-%H%M%S");

} // end of namespace nevistpc

#endif //_STRINGUTILS_H

