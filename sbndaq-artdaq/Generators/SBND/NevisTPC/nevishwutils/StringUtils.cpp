#include <algorithm>
#include <ctime>
#include "StringUtils.h"


namespace nevistpc {

char toupper(char i) {
    return std::toupper (i);
}

char tolower(char i) {
    return std::tolower (i);
}

std::string to_upper(std::string const & value)
{
    std::string line(value);

    transform(line.begin(), line.end(), line.begin(), toupper);

    return line;
}

std::string to_lower(std::string const & value)
{
    std::string line(value);

    transform(line.begin(), line.end(), line.begin(), tolower);

    return line;
}

std::string filterString(std::string const & value,
                         std::string const & pattern)
{
    std::string line(value);

    size_t index = line.find_first_of(pattern.c_str());

    if (index!=std::string::npos)
        line.resize(index);

    transform(line.begin(), line.end(), line.begin(), toupper);

    line = line.erase( line.find_last_not_of(" ") + 1);
    line = line.erase(0 , line.find_first_not_of(" ") );

    return line;
}

std::string getCmdOption(char ** begin, char ** end,  std::string const& option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return std::string(*itr);
    }
    return std::string();
}

bool cmdOptionExists(char** begin, char** end,  std::string const& option)
{
    return std::find(begin, end, option) != end;
}

std::string removeExtension(const std::string & filename) {
  size_t lastdot = filename.find_last_of(".");
  if (lastdot == std::string::npos) return filename;
  return filename.substr(0, lastdot);
}

std::string getTimestampAsString(std::string format )
{
  time_t now;
  char timestamp[80];
  
  timestamp[0] = '\0';
  
  now = time(NULL);
  
  if(now != -1)
  {
    strftime(timestamp, 80, format.c_str(), gmtime(&now));
  }
  
  return std::string(timestamp);
}

} // end of namespace nevistpc
