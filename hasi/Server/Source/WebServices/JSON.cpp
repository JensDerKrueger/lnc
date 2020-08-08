#include <algorithm>

#include "JSON.h"
#include <Tools/SysTools.h>

using namespace IVDA::SysTools;

static std::string escape(std::string s) {
  ReplaceAll(s, "\\", "\\\\");
  ReplaceAll(s, "\"", "\\\"");
  ReplaceAll(s, "\b", "\\b");
  ReplaceAll(s, "\f", "\\f");
  ReplaceAll(s, "\n", "\\n");
  ReplaceAll(s, "\r", "\\r");
  ReplaceAll(s, "\t", "\\t");
  return s;
}

static std::string unescape(std::string s) {
  ReplaceAll(s, "\\\"", "\"");
  ReplaceAll(s, "\\b", "\b");
  ReplaceAll(s, "\\f", "\f");
  ReplaceAll(s, "\\n", "\n");
  ReplaceAll(s, "\\r", "\r");
  ReplaceAll(s, "\\t", "\t");
  ReplaceAll(s, "\\\\", "\\");
  return s;
}

static std::string trimValue(std::string& s, const std::string& altEnd);

static void strMove(std::string& source, std::string&  target, size_t count) {
  target += source.substr(0, count);
  source = source.substr(count);
}

static std::string trimStr(std::string& s) {
  s = TrimStr(s);

  if (s[0] != '"')
    throw JSONParseException(std::string("Expected string here (no opening"
                                         " quotation mark found) ->") + s);
  size_t iPos = 0;
  while (iPos+1 < s.length() && s[iPos+1] != '"') {
    iPos++;
    if (s[iPos] == '\\') iPos++;
  }
  
  if (iPos+1 >= s.length())
    throw JSONParseException(std::string("Expected string here (no closing"
                                         " quotation mark found) ->") + s);
  
  std::string result = s;
  s = s.substr(iPos+2);
  
  return result.substr(0, iPos+2);
}

static std::string trimArray(std::string& s) {
  std::string result = "";
  s = TrimStr(s);

  if (s[0] != '[') {
    throw JSONParseException(std::string("Expected array"
                                         " here (squared brace) ->") + s);
  }

  do {
    strMove(s, result, 1);
    result += trimValue(s, "]");
    s = TrimStrLeft(s);
    if (s[0] != ',' && s[0] != ']') {
      throw JSONParseException(std::string("Expected comma"
                                           " or squared brace here ->") + s);
    }
  } while (s[0] == ',');
  strMove(s, result, 1);
  s = TrimStrLeft(s);
  return result;
}

static std::string trimObj(std::string& s) {
  std::string result = "";
  s = TrimStr(s);
  
  if (s[0] != '{')
    throw JSONParseException(std::string("Expected object"
                                         " here (curly brace) ->") + s);
  do {
    strMove(s, result, 1);

    s = TrimStrLeft(s);
    if (s[0] == '}') {
      break; // empty object found
    }
    result += trimStr(s);
    s = TrimStrLeft(s);

    if (s[0] != ':')
      throw JSONParseException(std::string("Expected colon here ->") + s);

    strMove(s, result, 1);

    result += trimValue(s, "}");
    s = TrimStrLeft(s);

    if (s[0] != ',' && s[0] != '}')
      throw JSONParseException(std::string("Expected comma"
                                           " or curly brace here ->") + s);

  } while (s[0] == ',');
  strMove(s, result, 1);
  s = TrimStrLeft(s);

  return result;
}

static std::string trimValue(std::string& s, const std::string& altEnd) {
  s = TrimStr(s);

  if (s[0] == '"')
    return trimStr(s);
  if (s[0] == '[')
    return trimArray(s);
  if (s[0] == '{')
    return trimObj(s);
  std::size_t found = std::min(s.find_first_of(","), s.find_first_of(altEnd));

  std::string result = s;
  if (found!=std::string::npos) {
    s = s.substr(found);
    return result.substr(0,found);
  }

  throw JSONParseException(std::string("Expected comma or ")
                           + altEnd + std::string(" here ->") + s);
}

JSONValue::JSONValue(const std::string& v) {
  strValue = v;
  valueType = JVT_STR;
}

JSONValue::JSONValue(int32_t v){
  iValue = v;
  valueType = JVT_INT;
}

JSONValue::JSONValue(double v){
  fValue = v;
  valueType = JVT_FLOAT;
}

JSONValue::JSONValue(JSONObjectPtr v){
  oValue = v;
  valueType = JVT_OBJ;
}

JSONValue::JSONValue(const std::vector<JSONValuePtr>& v){
  aValue = v;
  valueType = JVT_ARRAY;
}

JSONValue::JSONValue(eJSONConst v){
  switch (v) {
    case JC_TRUE:
      valueType = JVT_TRUE;
      break;
    case JC_FALSE:
      valueType = JVT_FALSE;
      break;
    case JC_NULL:
      valueType = JVT_NULL;
      break;
  }
}

std::string JSONValue::serialize() const {
  switch (valueType) {
    case JVT_STR:
      return std::string("\"") + escape(strValue) + std::string("\"");
    case JVT_INT:
      return ToString(iValue);
    case JVT_FLOAT:
      return ToString(fValue);
    case JVT_OBJ:
      return oValue->serialize();
    case JVT_ARRAY: {
      std::stringstream ss;
      ss << "[";
      for (size_t i = 0;i<aValue.size();++i) {
        ss << aValue[i]->serialize();
        if (i < aValue.size()-1) ss << ",";
      }
      ss << "]";
      return ss.str();
    }
    case JVT_TRUE:
      return "true";
    case JVT_FALSE:
      return "false";
    default : // case JVT_NULL:
      return "null";
  }

}


void JSONValue::deSerializeArray(std::string s) {
  s = TrimStr(s);
  if (s.empty() || s[0] != '[') {
    throw JSONParseException(std::string("Expected array"
                                         " here (squared brace) ->") + s);
  }

  s = s.substr(1);
  do {
    std::string valueStr = trimValue(s,"]");
    JSONValuePtr v = std::make_shared<JSONValue>();
    v->deSerialize(valueStr);
    aValue.push_back(v);
    s = TrimStr(s);
    if (s[0]==',' || s[0]==']') s = s.substr(1);
  } while (!s.empty());
  valueType = JVT_ARRAY;
}

void JSONValue::deSerialize(std::string s) {
  s = TrimStr(s);
  std::string us = ToUpperCase(s);

  if (us == "TRUE") {
    valueType = JVT_TRUE;
    return;
  }
  if (us == "FALSE") {
    valueType = JVT_FALSE;
    return;
  }
  if (us == "NULL") {
    valueType = JVT_NULL;
    return;
  }
  if (s[0] == '"' && s[s.length()-1] == '"') {
    strValue = unescape(s.substr(1,s.length()-2));
    valueType = JVT_STR;
    return;
  }
  if (s[0] == '[' && s[s.length()-1] == ']') {
    deSerializeArray(s);
    return;
  }
  if (s[0] == '{' && s[s.length()-1] == '}') {
    oValue = std::make_shared<JSONObject>(s);
    valueType = JVT_OBJ;
    return;
  }
  if (ToString(FromString<int32_t>(s)) == s ) {
    iValue = FromString<int32_t>(s);
    valueType = JVT_INT;
    return;
  }
  
  // TODO: check if s is a valid float representation

  fValue = FromString<float>(s);
  valueType = JVT_FLOAT;
}

JSONObject::JSONObject(const std::string& source) {
  deSerialize(source);
}

JSONObject::JSONObject(const std::string& name, const JSONValuePtr& value)
{
  addValue(name, value);
}

void JSONObject::addValue(const std::string& name, const JSONValuePtr& value) {
  m_data.push_back(std::make_pair(name,value));
}

void JSONObject::removeValue(size_t i) {
  m_data.erase(m_data.begin()+i);
}

std::string JSONObject::serialize() const {
  std::stringstream ss;
  ss.precision(15);
  ss << "{";
  for (size_t i = 0;i<m_data.size();++i) {
    ss << "\"" << escape(m_data[i].first)
       << "\":" << m_data[i].second->serialize();
    if (i < m_data.size()-1) ss << ",";
  }
  ss << "}";
  return ss.str();
}


void JSONObject::deSerialize(std::string s) {
  m_data.clear();

  s = TrimStr(s);

  if (s[0] != '{') {
    throw JSONParseException(std::string("Expected object"
                                         " here (curly brace) ->") + s);
  }

  do {
    s = s.substr(1);

    std::string section, propName;
    // empty object
    if (s[0] == '}') {
      section = "";
      propName = "";
    } else {
      section = trimStr(s);
      propName = unescape(section.substr(1,section.length()-2));
      s = TrimStrLeft(s);
      if (s[0] != ':')
        throw JSONParseException(std::string("Expected colon here ->") + s);
      s = s.substr(1);
    }
    section = trimValue(s, "}");
    JSONValuePtr propValue = std::make_shared<JSONValue>();
    propValue->deSerialize(section);
    s = TrimStrLeft(s);
    if (s[0] != ',' && s[0] != '}')
      throw JSONParseException(std::string("Expected comma"
                                           " or curly brace here ->") + s);
    
    m_data.push_back(std::make_pair(propName, propValue));
  } while (s[0] == ',');
  s = s.substr(1);
  s = TrimStrLeft(s);

  if (!s.empty()) {
    throw JSONParseException(std::string("Unexpected data found here ->") + s);
  }
}

const JSONValuePtr JSONObject::getValue(const std::string& name) const {
  for (size_t i = 0;i<m_data.size();++i) {
    if (m_data[i].first == name) return m_data[i].second;
  }
  return nullptr;
}

/*
 The MIT License
 
 Copyright (c) 2014 Jens Krueger
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */
