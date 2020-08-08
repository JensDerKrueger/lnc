#ifndef EXPRESSIONEXCEPTION_H
#define EXPRESSIONEXCEPTION_H

#include <exception>
#include <string>

/*
 class ParseException : public std::runtime_error {
public:
  explicit ParseException (const std::string& what_arg) :
  std::runtime_error(what_arg) {}
  explicit ParseException (const char* what_arg) :
  std::runtime_error(what_arg) {}
};
*/

class StrException : public std::exception {
public:
  StrException(const std::string& m) : msg(m) {}
  ~StrException() throw() {}
  const char* what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};

class ParseException : public StrException {
public:
  ParseException(const std::string& m) : StrException(m) {}
};

class ExecuteException : public StrException {
public:
  ExecuteException(const std::string& m) : StrException(m) {}
};

#endif// EXPRESSIONEXCEPTION_H