#include "StringTools.h"

#include <algorithm>
#include <cstring>
#include <cctype>

std::string trimLeft(const std::string& input) {
  std::string result = input;
  result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {return !std::isspace(ch);}));
  return result;
}

std::string trimRight(const std::string& input) {
  std::string result = input;
  result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), result.end());
  return result;
}

std::string trim(const std::string& input) {
  return trimLeft(trimRight(input));
}

std::string toLower(const std::string& input) {
  std::string result = input;
  std::transform(input.begin(), input.end(), result.begin(), [](int c) { return std::tolower(c); });
  return result;
}

std::string toUpper(const std::string& input) {
  std::string result = input;
  std::transform(input.begin(), input.end(), result.begin(), [](int c) { return std::toupper(c); });
  return result;
}

std::vector<std::string> tokenize(const std::string& input, const std::string& delim) {
  char* cstring = strdup(input.c_str());
  char* token = std::strtok(cstring, delim.c_str());
  std::vector<std::string> result;
  while (token) {
    result.push_back(token);
    token = std::strtok(nullptr,  delim.c_str());
  }
  free(cstring);
  return result;
}

bool endsWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}
