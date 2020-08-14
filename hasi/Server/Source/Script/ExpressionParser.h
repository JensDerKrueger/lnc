#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "Operator.h"
#include "Primary.h"
#include "ExpressionExceptions.h"
#include "CommandLine.h"

class Template {
public:
  Template(const std::vector<std::string>& templateLines);
  
  std::string getName() const;
  size_t getParamCount() const;
  std::vector<std::string> resolvedCode(const std::vector<std::string>& params) const;

  std::string toString() const;
  
private:
  std::string m_name;
  std::vector<std::string> m_params;
  std::vector<std::string> m_codeLines;
  
};

class Alias {
public:
  Alias(const std::string& aliasLine);
  
  std::string getName() const;
  std::string getReplacement() const;
  
  std::string toString() const;
  
private:
  std::string m_name;
  std::string m_replacement;
  
};

class ExpressionParser {
public:
  ExpressionParser(const std::string& filename, uint32_t iTemplateRecLimit=100);

  void testParse() const;
  void testParse(const std::string& filename) const;
  
  void reParse(bool bApply);
  void reParse(const std::string& filename, bool bApply);
  std::vector<CommandPtr> execute(const Variables& triggerer,
                                  const VarAssignments& va) const;

  std::string toString() const;

  Variables getTriggers() const;
  Variables getVariables() const;
  Outputs getOutputs() const;

  static ExpressionPtr parseArithmeticExpression (std::string expression);
  static void ParseRHS(const std::string& str, CommandVec& cv);
  static void ParseRHS(const std::string& str, CommandVec& cv,
                       const VarAssignments& va);
  
private:
  std::string filename;
  uint32_t m_iTemplateRecLimit;
  std::vector<CommandLinePtr> cmdLines;
  
  CommandLinePtr parseLine(const std::string& line);
  
  static std::string extractSubExpression(std::string& expression);
  static ExpressionPtr parsePrimary(std::string& expression);
  static Operator parseOp(std::string& expression);
  static ExpressionPtr parseArithmeticExpressionRec(ExpressionPtr lhs,
                                                    int min_precedence,
                                                    std::string& expression);
  static void findTemplates(std::vector<std::string>& lines,
                            std::vector<std::shared_ptr<Template>>& templates);
  
  static void resolveTemplates(std::vector<std::string>& lines,
                               const std::vector<std::shared_ptr<Template>>& templates,
                               uint32_t iMaxDepth, uint32_t iDepth);
  
  static void findAliases(std::vector<std::string>& lines,
                          std::vector<std::shared_ptr<Alias>>& aliases);

  static void resolveAliases(std::vector<std::string>& lines,
                             const std::vector<std::shared_ptr<Alias>>& aliases);
  
  static std::vector<std::string> resolveTemplate(const std::string& name,
                                                  const std::vector<std::string>& params,
                                                  const std::vector<std::shared_ptr<Template>>& templates);

  static bool parseTemplateCall(const std::string& line,
                                std::string& name,
                                std::vector<std::string>& params);
  
  void reParse(const std::vector<std::string>& lines, bool bApply);
  
  std::vector<std::string> fileToLineVector(const std::string& filename);
  std::vector<std::string> fileToLineVector(const std::string& filename,
                                            std::vector<std::string>& includedFiles);
  
};

#endif // EXPRESSIONPARSER_H
