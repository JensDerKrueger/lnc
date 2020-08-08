#ifndef JSON_H
#define JSON_H

#include <memory>
#include <string>       // std::string
#include <vector>

class JSONParseException : public std::exception {
public:
  JSONParseException(const std::string& m) : msg(m) {}
  JSONParseException() throw() {}
  virtual ~JSONParseException() throw() {}
  const char* what() const throw() { return msg.c_str(); }
private:
  std::string msg;
};

class JSONObject;
typedef std::shared_ptr<JSONObject> JSONObjectPtr;

class JSONValue;
typedef std::shared_ptr<JSONValue> JSONValuePtr;

class JSONValue {
public:

  enum eJSONConst {
    JC_TRUE,
    JC_FALSE,
    JC_NULL
  };

  JSONValue(const std::string& v);
  JSONValue(int32_t v);
  JSONValue(double v);
  JSONValue(JSONObjectPtr v);
  JSONValue(const std::vector<JSONValuePtr>& v);
  JSONValue(eJSONConst v=JC_NULL);

  enum eJSONValueType {
    JVT_STR,
    JVT_INT,
    JVT_FLOAT,
    JVT_OBJ,
    JVT_ARRAY,
    JVT_TRUE,
    JVT_FALSE,
    JVT_NULL
  } valueType;

  std::string strValue;
  int32_t iValue;
  double fValue;
  JSONObjectPtr oValue;
  std::vector<JSONValuePtr> aValue;

  std::string serialize() const;
  void deSerialize(std::string s);

private:
  void deSerializeArray(std::string s);
};

class JSONObject  {
public:
  JSONObject(const std::string& source);
  JSONObject(const std::string& name, const JSONValuePtr& value);
  void addValue(const std::string& name, const JSONValuePtr& value);
  void removeValue(size_t i);

  const std::string& getName(size_t i=0) const {return m_data[i].first;}
  const JSONValuePtr getValue(size_t i=0) const {return m_data[i].second;}
  const JSONValuePtr getValue(const std::string& name) const;
  size_t getValueCount() const {return m_data.size();}

  std::string serialize() const;
  void deSerialize(std::string s);

protected:
  std::vector<std::pair<std::string, JSONValuePtr>> m_data;

};

#endif // JSON_H


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
