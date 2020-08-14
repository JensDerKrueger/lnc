#ifndef PARSERTOOLS_H
#define PARSERTOOLS_H

#include <vector>
#include <string>
#include <sstream>

namespace ParserTools {
  void tokenize(const std::string& str,
                std::vector<std::string>& tokens,
                const std::string& delimiters = " ",
                bool skipEmpty = true);
  std::string removeComments(const std::string& str,
                             const std::string& commentSigns = "#");
  std::string removeSpaces(const std::string& expression);
  std::string toLowerCase(const std::string& str);
  
  bool startsWith(const std::string& s, const std::string& prefix,
                  std::string& tail);

  bool startsWith(const std::string& s, const std::string& prefix);
  
  template<class T> T fromString(const std::string& s) {
    std::istringstream stream (s);
    T t;
    stream >> t;
    return t;
  }
  
  bool charCheck(char c, char min=0x20, char max=0x7E);
  size_t stringCheck(const std::string& s, char min=0x20, char max=0x7E);
  
}

#endif // PARSERTOOLS_H
