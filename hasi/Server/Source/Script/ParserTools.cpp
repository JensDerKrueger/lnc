#include "ParserTools.h"

#include <algorithm>

bool ParserTools::charCheck(char c, char min, char max) {
  return c >= min && c <= max;
}

size_t ParserTools::stringCheck(const std::string& s, char min, char max) {
  for(size_t i = 0; i < s.length(); ++i) {
    const char& c = s[i];
    if (!charCheck(c)) return i;
  }
  return std::string::npos;
}

void ParserTools::tokenize(const std::string& str,
                           std::vector<std::string>& tokens,
                           const std::string& delimiters,
                           bool skipEmpty)
{
  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  
  // Find first "non-delimiter".
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
  
  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    if (skipEmpty)
      lastPos = str.find_first_not_of(delimiters, pos);
    else
      lastPos = (pos == std::string::npos) ? std::string::npos : pos+1 ;
    
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

std::string ParserTools::removeComments(const std::string& str,
                                       const std::string& commentSigns) {
  std::string::size_type pos = str.find_first_of(commentSigns, 0);
  
  if (pos == 0) return "";
  
  if (std::string::npos != pos) {
    return str.substr(0,pos-1);
  } else {
    return str;
  }
}

std::string ParserTools::removeSpaces(const std::string& expression) {
  std::string result;
  for (size_t i = 0;i<expression.length();++i) {
    if (!isspace(expression[i])) {
      result += expression[i];
    }
  }
  return result;
}

inline int myTolower(int c) {return tolower(static_cast<unsigned char>(c));}

std::string ParserTools::toLowerCase(const std::string& str) {
  std::string result(str);
  std::transform(str.begin(), str.end(), result.begin(), myTolower);
  return result;
}

bool ParserTools::startsWith(const std::string& s, const std::string& prefix,
                             std::string& tail) {
  bool b = (s.length() >= prefix.length()) &&
              s.substr(0,prefix.length()) == prefix;
  if (b) {
    tail = s.substr(prefix.length());
  }
  return b;
}

bool ParserTools::startsWith(const std::string& s, const std::string& prefix) {
  return (s.length() >= prefix.length()) &&
            s.substr(0,prefix.length()) == prefix;
}

