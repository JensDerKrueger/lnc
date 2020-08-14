#include "ExpressionParser.h"

#include <cctype>
#include <sstream>
#include <fstream>
#include <algorithm>    // std::find

#include "CompoundExpression.h"
#include "Variable.h"
#include "Value.h"
#include "Command.h"
#include "ParserTools.h"

#include <Tools/DebugOutHandler.h>  // IVDA_MESSAGE
#include <Tools/SysTools.h> // ReplaceAll

// #define DEBUG_OUT

#ifdef DEBUG_OUT
#include <iostream>
#endif

#include <iostream>

Template::Template(const std::vector<std::string>& templateLines) {
  if (templateLines.empty()) {
    std::stringstream ss;
    ss << "Unable to create template from empty lines";
    throw ParseException(ss.str());
  }
  
  const std::string& line = templateLines[0];
  
  size_t paramStart = line.find("(");
  size_t paramEnd = line.find(")");

  if (paramStart == std::string::npos) {
    std::stringstream ss;
    ss << "Improper parenthesis (missing opening braket) for template " << line;
    throw ParseException(ss.str());
  }

  if (paramEnd == std::string::npos) {
    std::stringstream ss;
    ss << "Improper parenthesis (missing closing braket) for template " << line;
    throw ParseException(ss.str());
  }

  if (paramStart > paramEnd) {
    std::stringstream ss;
    ss << "Improper parenthesis (brakets out of order) for template " << line;
    throw ParseException(ss.str());
  }
 
  if (paramEnd != line.length()-1) {
    std::stringstream ss;
    ss << "Improper parenthesis (line does not end with braket) for template " << line;
    throw ParseException(ss.str());
  }
  
  
  m_name = line.substr(0,paramStart);
  ParserTools::tokenize(line.substr(paramStart+1,paramEnd-(paramStart+1)), m_params, ",");

  for (size_t i = 0;i<m_params.size();++i) {
    m_params[i] = std::string("[")+m_params[i]+std::string("]");
  }
  
  m_codeLines.insert(m_codeLines.end(), templateLines.begin()+1, templateLines.end());
}

std::string Template::getName() const {
  return m_name;
}

size_t Template::getParamCount() const {
  return m_params.size();
}

std::vector<std::string> Template::resolvedCode(const std::vector<std::string>& params) const {
  std::vector<std::string> newCode;

  for (auto line = m_codeLines.begin(); line != m_codeLines.end();++line) {
    std::string targetString = *line;
    
    for (size_t i = 0;i<m_params.size();++i) {
      IVDA::SysTools::ReplaceAll(targetString, m_params[i], params[i]);
    }
    
    newCode.push_back(targetString);
  }
  
  return newCode;
}

std::string Template::toString() const {
  std::stringstream ss;
  ss << std::endl << "Template " << m_name << std::endl;
  ss << "  Parameters:" << std::endl;
  for (auto p = m_params.begin(); p != m_params.end();++p) {
      ss << "    " << *p << std::endl;
  }
  ss << "  Code:" << std::endl;
  for (auto l = m_codeLines.begin(); l != m_codeLines.end();++l) {
    ss << "    " << *l << std::endl;
  }
  ss << "End" << std::endl;
  
  return ss.str();
}


Alias::Alias(const std::string& line) {
  if (line.empty()) {
    std::stringstream ss;
    ss << "Unable to create alias from empty line";
    throw ParseException(ss.str());
  }
  
  size_t paramStart = line.find("(");
  size_t paramEnd = line.find(")");
  
  if (paramStart == std::string::npos) {
    std::stringstream ss;
    ss << "Improper parenthesis (missing opening braket) for alias " << line;
    throw ParseException(ss.str());
  }
  
  if (paramEnd == std::string::npos) {
    std::stringstream ss;
    ss << "Improper parenthesis (missing closing braket) for alias " << line;
    throw ParseException(ss.str());
  }
  
  if (paramStart > paramEnd) {
    std::stringstream ss;
    ss << "Improper parenthesis (brakets out of order) for alias " << line;
    throw ParseException(ss.str());
  }
  
  if (paramEnd != line.length()-1) {
    std::stringstream ss;
    ss << "Improper parenthesis (line does not end with braket) for alias " << line;
    throw ParseException(ss.str());
  }
  
  
  std::vector<std::string> params;
  ParserTools::tokenize(line.substr(paramStart+1,paramEnd-(paramStart+1)), params, ";");

  if (params.size() != 2) {
    std::stringstream ss;
    ss << "Invalid alias parameters '" << line << "' expected two words.";
    throw ParseException(ss.str());
  }
  
  m_name = params[0];
  m_replacement = params[1];
}

std::string Alias::getName() const {
  return m_name;
}

std::string Alias::getReplacement() const {
  return m_replacement;
}

std::string Alias::toString() const {
  std::stringstream ss;
  ss << std::endl << "Alias " << m_name << "->" << getReplacement() << std::endl;
  return ss.str();
}

ExpressionParser::ExpressionParser(const std::string& _filename,
                                   uint32_t iTemplateRecLimit) :
filename(_filename),
m_iTemplateRecLimit(iTemplateRecLimit)
{
  reParse(true);
}

void ExpressionParser::reParse(bool bApply) {
  reParse(filename, bApply);
}

void ExpressionParser::findTemplates(std::vector<std::string>& lines,
                                     std::vector<std::shared_ptr<Template>>& templates) {
  
  std::vector<std::string> templateLines;
  std::vector<std::string> templateFreeLines;
  
  for (size_t i = 0;i<lines.size();++i) {
    std::string processedLine = lines[i];
    
    std::string tail;
    if (ParserTools::startsWith(processedLine, "template", tail)) {
      templateLines.push_back(tail);
    } else {
      if (!templateLines.empty()) {
        // we are inside a template
        if (processedLine == "end") {
          templates.push_back(std::make_shared<Template>(templateLines));
          templateLines.clear();
        } else {
          templateLines.push_back(processedLine);
        }
      } else {
        // we are parsing regular code
        templateFreeLines.push_back(processedLine);
      }
    }
  }
  
  if (!templateLines.empty()) {
    std::stringstream ss;
    ss << "Missing end statement for template \"" << templateLines[0] << "\"";
    throw ParseException(ss.str());
  }

  if (!templates.empty() && templates[templates.size()-1]->getName() == "alias") {
    std::stringstream ss;
    ss << "'alias' is a reserved keyword and cannot be used as a template name";
    throw ParseException(ss.str());
  }
  
  lines = templateFreeLines;
}


bool ExpressionParser::parseTemplateCall(const std::string& line,
                                         std::string& name,
                                         std::vector<std::string>& params ) {
  if (line.find(":") != std::string::npos) return false; // template calls must not contain a colon
  
  size_t paramStart = line.find("(");
  size_t paramEnd = line.rfind(")");
  
  if (paramStart == std::string::npos ||
      paramEnd == std::string::npos ||
      paramStart > paramEnd ||
      paramEnd != line.length()-1) {
    std::stringstream ss;
    ss << "Invalid colon count or malformed template in line \"" << line << "\"";
    throw ParseException(ss.str());
  }
  
  name = line.substr(0,paramStart);
  ParserTools::tokenize(line.substr(paramStart+1,paramEnd-(paramStart+1)), params, ";");
  
  return true;
}


std::vector<std::string> ExpressionParser::resolveTemplate(const std::string& name,
                                                           const std::vector<std::string>& params,
                                                           const std::vector<std::shared_ptr<Template>>& templates) {
  std::shared_ptr<Template> t = nullptr;

  for (auto temp = templates.begin(); temp != templates.end(); ++temp) {
    if ((*temp)->getName() == name && (*temp)->getParamCount() == params.size()) {
      t = *temp;
      break;
    }
  }
  
  if (t) {
    return t->resolvedCode(params);
  } else {
    std::stringstream ss;
    ss << "Template " << name << " with " << params.size() << " parameter(s) not found.";
    throw ParseException(ss.str());
  }
}

void ExpressionParser::resolveTemplates(std::vector<std::string>& lines,
                                        const std::vector<std::shared_ptr<Template>>& templates,
                                        uint32_t iMaxDepth, uint32_t iDepth) {

  if (iDepth > iMaxDepth) {
    std::stringstream ss;
    ss << "Recursion limit reached while resolving templates";
    throw ParseException(ss.str());
  }
  
  bool bFound = false;
  std::vector<std::string> templatedLines;
  
  for (size_t i = 0;i<lines.size();++i) {
    std::string processedLine = lines[i];

    std::string name;
    std::vector<std::string> params;
    if (parseTemplateCall(processedLine, name, params)) {
      std::vector<std::string> newlines = resolveTemplate(name, params, templates);
      templatedLines.insert(templatedLines.end(), newlines.begin(), newlines.end());
      bFound= true;
    } else {
      templatedLines.push_back(processedLine);
    }
  }
  
  
  if (bFound) {
    resolveTemplates(templatedLines, templates, iMaxDepth, iDepth+1);
    lines = templatedLines;
  }

}

void ExpressionParser::findAliases(std::vector<std::string>& lines,
                                   std::vector<std::shared_ptr<Alias>>& aliases) {
  
  std::vector<std::string> aliasFreeLines;
  
  for (size_t i = 0;i<lines.size();++i) {
    std::string tail;
    if (ParserTools::startsWith(lines[i], "alias", tail)) {
      aliases.push_back(std::make_shared<Alias>(tail));
    } else {
      aliasFreeLines.push_back(lines[i]);
    }
  }
  
  lines = aliasFreeLines;
}

void ExpressionParser::resolveAliases(std::vector<std::string>& lines,
                                      const std::vector<std::shared_ptr<Alias>>& aliases) {
  for (size_t i = 0;i<lines.size();++i) {
    for (auto a : aliases) {
      const std::string s = a->getName();
      const std::string t = a->getReplacement();
      if (lines[i].find(s) != std::string::npos) {
        IVDA::SysTools::ReplaceAll(lines[i], s, t);
      }
    }
  }
}

std::vector<std::string> ExpressionParser::fileToLineVector(const std::string& filename) {
  std::vector<std::string> includedFiles;
  return fileToLineVector(filename, includedFiles);
}

std::vector<std::string> ExpressionParser::fileToLineVector(const std::string& filename,
                                                            std::vector<std::string>& includedFiles) {
  std::vector<std::string> lines;
  
  if (std::find (includedFiles.begin(), includedFiles.end(), filename) != includedFiles.end()) {
    // recursive include
    return lines;
  }
  
  if (includedFiles.size() > 200) {
    throw ParseException("Too many includes to process file.");
  }
  includedFiles.push_back(filename);
  
  std::ifstream file(filename);
  std::string line;
  
  while (std::getline(file, line))
  {
    std::string processedLine = ParserTools::removeComments(IVDA::SysTools::TrimStrLeft(line));

    if (line.find("//") != std::string::npos ||
        line.find("/*") != std::string::npos ||
        line.find("*/") != std::string::npos) {
      IVDA_WARNING("Found c-style comment line \""+line+"\" ignoring line");
      continue;
    }
    
    if (!line.empty() && line[line.length()-1] == ';') {
      IVDA_WARNING("Found c-style semicolon at end of line \""+line+"\" ignoring line");
      continue;
    }

    // replace tabs with spaces for the following check
    IVDA::SysTools::ReplaceAll(processedLine, "\t", " ");
    
    size_t iProblemPos;
    while (std::string::npos != (iProblemPos = ParserTools::stringCheck(processedLine))) {
      std::string markerStr;
      std::stringstream prefixTextStream;
      prefixTextStream << "Found illegal char " << uint32_t(*reinterpret_cast<uint8_t*>(&processedLine[iProblemPos])) << " outside of comment in line \"";
      
      for (size_t i = 0;i<prefixTextStream.str().length()+iProblemPos;++i) markerStr += " ";
      markerStr += "^ (replaced it with a space)";
      
      IVDA_WARNING(prefixTextStream.str() << processedLine << "\"");
      IVDA_WARNING(markerStr);
      processedLine[iProblemPos] = ' ';
    }
        
    std::string tail;
    if (ParserTools::startsWith(processedLine, "include", tail)) {
      std::vector<std::string> includedLines = fileToLineVector(IVDA::SysTools::TrimStr(tail), includedFiles);
      lines.insert(lines.end(), includedLines.begin(), includedLines.end());
    } else {
      processedLine = ParserTools::removeSpaces(processedLine);
      if (processedLine.empty()) continue;
      lines.push_back(processedLine);
    }
    
  }
  
  return lines;
}


void ExpressionParser::reParse(const std::string& _filename, bool bApply) {
  std::vector<std::string> lines = fileToLineVector(_filename);
  
  std::vector<std::shared_ptr<Alias>> aliases;
  findAliases(lines, aliases);
  resolveAliases(lines, aliases);

  std::vector<std::shared_ptr<Template>> templates;
  findTemplates(lines, templates);
  resolveTemplates(lines, templates, m_iTemplateRecLimit, 0);
  
  reParse(lines, bApply);
  
  if (bApply)
    filename = _filename;
}

void ExpressionParser::reParse(const std::vector<std::string>& lines, bool bApply) {
  std::vector<CommandLinePtr> newCmdLines;
  
  for (auto line = lines.begin(); line != lines.end(); ++line) {
    std::string processedLine = *line;
    
    CommandLinePtr c = parseLine(processedLine);
    if (c->getTriggers().empty()) {
      std::stringstream ss;
      ss << "The simplified expression \"" << c->getExpression()->toString() << "\" in line \"" << *line << "\" is constant!";
      throw ParseException(ss.str());
    }
    newCmdLines.push_back(c);
  }
  
  if (bApply) cmdLines = newCmdLines;
}


void ExpressionParser::ParseRHS(const std::string& str, CommandVec& cv,
                                const VarAssignments& vas) {
  ParseRHS(str, cv);
  for (size_t i = 0;i<cv.size();++i) {
    cv[i] = cv[i]->evaluate(vas);
  }
}

void ExpressionParser::ParseRHS(const std::string& str, CommandVec& cv) {
  std::vector<std::string> cmdStrings;
  ParserTools::tokenize(str,cmdStrings, ",");
  
  for (std::vector<std::string>::iterator i = cmdStrings.begin();
       i != cmdStrings.end();
       ++i) {
    cv.push_back(Command::StringToCommand(*i));
  }
}


CommandLinePtr ExpressionParser::parseLine(const std::string& line) {
  std::vector<std::string> parts;
  ParserTools::tokenize(line, parts, ":");
  
  if (parts.size() != 2) {
    throw ParseException(std::string("invalid colon count in line \"") + line + std::string("\""));
  }
  
  ExpressionPtr ex = parseArithmeticExpression(parts[0]);
  CommandVec cv;
  ParseRHS(parts[1], cv);
  
  return CommandLinePtr(new CommandLine(ex,cv));
}

std::string ExpressionParser::extractSubExpression(std::string& expression) {
  uint32_t iCounter = 1;
  uint32_t iPos = 1;
  for (iPos = 1; iPos < expression.length(); iPos++) {
    if (expression[iPos] == ')') iCounter--;
    if (expression[iPos] == '(') iCounter++;

    if (iCounter == 0) break;
  }

  if (iCounter != 0) {
    std::stringstream ss;
    ss << "invalid parenthesis (" << iCounter << ") in expression \"" << expression << "\"";
    throw ParseException(ss.str());
  }

  std::string subExpression = expression.substr(1,iPos-1);
  expression = expression.substr(iPos+1);


  return subExpression;
}

ExpressionPtr ExpressionParser::parsePrimary(std::string& expression) {
  /*
   if (expression.empty())
   std::cout << "empty primary expression" << std::endl;
   else
   std::cout << "parsePrimary: " << expression << std::endl;
   */

  std::string primaryStr;

  if (expression.length() > 0 && expression[0] == '(') {
    std::string subExpression = extractSubExpression(expression);
    return parseArithmeticExpression(subExpression);
  }


  if (expression.length() > 0 && (expression[0] == '-' || expression[0] == '+')) {
    primaryStr += expression[0];
    expression.erase(0,1);
  }

  while (expression.length() > 0 && (isdigit(expression[0]) || expression[0]=='.')) {
    primaryStr += expression[0];
    expression.erase(0,1);
  }

  if (primaryStr.empty()) {
    while (expression.length() > 0) {
      char c = expression[0];
      primaryStr += c;
      expression.erase(0,1);
      if (c == ']') break;
    }
  }

  return Primary::StringToPrimary(primaryStr);
}

Operator ExpressionParser::parseOp(std::string& expression) {
  /*
   if (expression.empty())
   std::cout << "empty op expression" << std::endl;
   else
   std::cout << "parseOp: " << expression << std::endl;
   */

  std::string str;
  while (expression.length() > 0 && Operator::isValidOperator(str+expression[0])) {
    str += expression[0];
    expression.erase(0,1);
  }

  if (str.empty() && !expression.empty()) {
    throw ParseException(std::string("invalid op at the beginning of \"") + expression + std::string("\""));
  }

  return Operator(str);
}

ExpressionPtr ExpressionParser::parseArithmeticExpression (std::string expression) {
  return parseArithmeticExpressionRec(parsePrimary(expression), 0, expression);
}


ExpressionPtr ConstructCompoundExpression(ExpressionPtr lhs,
                                          ExpressionPtr rhs,
                                          const Operator& op) {
  CompoundExpression cp = CompoundExpression(lhs, rhs, op);    
  VarAssignments va;
  return cp.evaluate(va);
}


ExpressionPtr ExpressionParser::parseArithmeticExpressionRec (ExpressionPtr lhs, int min_precedence, std::string& expression) {
  if (!expression.empty()) {
    Operator nextOp = parseOp(expression);
    
    while (!expression.empty() && nextOp.getPrecedence() >= min_precedence) {
      Operator op = nextOp;
      ExpressionPtr rhs = parsePrimary(expression);
      
      nextOp = parseOp(expression);
      
      while (!expression.empty() && nextOp.getPrecedence() > op.getPrecedence()) {
        Operator lookahead = nextOp;
        expression = lookahead.toString()+expression;
        rhs = parseArithmeticExpressionRec(rhs, lookahead.getPrecedence(), expression);
        nextOp = parseOp(expression);
      }

      
      lhs = ConstructCompoundExpression(lhs, rhs, op);
    }

    if (nextOp.getPrecedence() < min_precedence)
      expression = nextOp.toString()+expression;
  }
  return lhs;
}


std::vector<CommandPtr> ExpressionParser::execute(const Variables& triggerer, const VarAssignments& va) const {
  std::vector<CommandPtr> cmnds;
  
  for (std::vector<CommandLinePtr>::const_iterator i = cmdLines.begin();
       i != cmdLines.end();
       i++) {
  
    // check if a trigger variable triggers this commandline
    // if not then skip this line
    const Variables& triggers = (*i)->getTriggers();
    bool bExecuteRequired = false;
    for (Variables::const_iterator t = triggerer.begin();
         t != triggerer.end();
         t++) {
      if (triggers.count(*t)) {
        bExecuteRequired = true;
        break;
      }
    }
    if (!bExecuteRequired) continue;
    
    
    ExpressionPtr x = (*i)->getExpression();
    ExpressionPtr evX = x->evaluate(va);

#ifdef DEBUG_OUT
    std::cout << "  executing " << (*i)->getExpression()->toString() << std::endl;
    std::cout << "    result " << evX->toString() << std::endl;
#endif
    
    
    Value* vX = dynamic_cast<Value*>(evX.get());
    
    if (!vX) {
      std::stringstream ss;
      ss << "incomplete variable assignment for \"" << evX->toString() << "\" expression \"" << x->toString() << "\".";
      throw ExecuteException(ss.str());
    }
    
    if (vX->getValue() != 0.0) {
      const CommandVec& cv = (*i)->getCommandVec();
      for (CommandVec::const_iterator c = cv.begin();
           c != cv.end();
           c++) {
        cmnds.push_back((*c)->evaluate(va));
      }
    }
    
  }
  
  return cmnds;
}


Variables ExpressionParser::getVariables() const {
  Variables v;
  
  for (std::vector<CommandLinePtr>::const_iterator i = cmdLines.begin();
       i != cmdLines.end();
       i++) {
    Variables vi = (*i)->getVariables();
    v.insert(vi.begin(), vi.end());
  }
  
  return v;
}

Outputs ExpressionParser::getOutputs() const {
  Outputs o;

  for (std::vector<CommandLinePtr>::const_iterator i = cmdLines.begin();
       i != cmdLines.end();
       i++) {
    Outputs oi = (*i)->getOutputs();
    o.insert(oi.begin(), oi.end());
  }

  return o;
}

Variables ExpressionParser::getTriggers() const {
  Variables v;
  
  for (std::vector<CommandLinePtr>::const_iterator i = cmdLines.begin();
       i != cmdLines.end();
       i++) {
    Variables vi = (*i)->getTriggers();
    v.insert(vi.begin(), vi.end());
  }
  
  return v;
}

std::string ExpressionParser::toString() const {
  std::stringstream ss;
  
  ss << std::endl << "Commands:" << std::endl;
  for (std::vector<CommandLinePtr>::const_iterator i = cmdLines.begin();
       i != cmdLines.end();
       i++) {
    ss << (*i)->toString() << std::endl;
  }
  
  ss << std::endl << "Triggers:" << std::endl;
  const Variables t = getTriggers();
  for (Variables::const_iterator i = t.begin();
       i != t.end();
       i++) {
    ss << i->toString() << " ";
  }

  ss << std::endl << "All Variables:" << std::endl;
  const Variables v = getVariables();
  for (Variables::const_iterator i = v.begin();
       i != v.end();
       i++) {
    ss << i->toString() << " ";
  }
  
  return ss.str();
}

