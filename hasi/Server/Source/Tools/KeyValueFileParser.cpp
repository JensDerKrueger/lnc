#include "KeyValueFileParser.h"

#include <algorithm>
#include <functional>
#include "SysTools.h"

using namespace std;
using namespace IVDA;

KeyValPair::KeyValPair() :
strKey(""),
wstrKey(L""),
strKeyUpper(""),
wstrKeyUpper(L""),
strValue(""),
wstrValue(L""),
strValueUpper(""),
wstrValueUpper(L"")
{}

KeyValPair::KeyValPair(const string& key, const string& value) :
strKey(key),
wstrKey(key.begin(), key.end()),

strValue(value),
wstrValue(value.begin(), value.end())
{
  vstrValue = SysTools::Tokenize(value);
  for (size_t i = 0;i<vstrValue.size();i++) {
		vwstrValue.push_back(wstring(vstrValue[i].begin(), vstrValue[i].end()));
  }
  FillDerivedData();
}

KeyValPair::KeyValPair(const wstring& key, const wstring& value) :
strKey(key.begin(), key.end()),
wstrKey(key),

strValue(value.begin(), value.end()),
wstrValue(value)
{
  vwstrValue = SysTools::Tokenize(value);
  for (size_t i = 0;i<vwstrValue.size();i++) {
		vstrValue.push_back(string(vwstrValue[i].begin(), vwstrValue[i].end()));
  }
  FillDerivedData();
}

void KeyValPair::FillDerivedData() {
  int      _iValue;
  uint32_t _uiValue;
  float    _fValue;
  
  
  for (size_t i = 0;i<vwstrValue.size();i++) {
		if (SysTools::FromString(_iValue, vstrValue[i]))
		  viValue.push_back(_iValue);
		else
		  viValue.push_back(0);
    
		if (SysTools::FromString(_uiValue, vstrValue[i]))
		  vuiValue.push_back(_uiValue);
		else
		  vuiValue.push_back(0);
    
		if (SysTools::FromString(_fValue, vstrValue[i]))
		  vfValue.push_back(_fValue);
		else
		  vfValue.push_back(0.0f);
  }
  
  if (vwstrValue.size() > 0) {
		iValue  = viValue[0];
		uiValue = vuiValue[0];
		fValue  = vfValue[0];
  } else {
		iValue  = 0;
		uiValue = 0;
		fValue  = 0.0f;
  }
  
  strKeyUpper  = SysTools::ToUpperCase(strKey);
  wstrKeyUpper = SysTools::ToUpperCase(wstrKey);
  strValueUpper  = SysTools::ToUpperCase(strValue);
  wstrValueUpper  = SysTools::ToUpperCase(wstrValue);
}


KeyValueFileParser::KeyValueFileParser(const string& strFilename,
                                       bool bStopOnEmptyLine,
                                       const string& strToken,
                                       const std::string& strEndToken)
{
  m_bFileReadable = ParseFile(strFilename, bStopOnEmptyLine,
                              strToken, strEndToken);
}

KeyValueFileParser::KeyValueFileParser(const wstring& wstrFilename,
                                       bool bStopOnEmptyLine,
                                       const wstring& wstrToken,
                                       const std::wstring& wstrEndToken)
{
  string strFilename(wstrFilename.begin(), wstrFilename.end());
  string strToken(wstrToken.begin(), wstrToken.end());
  string strEndToken(wstrEndToken.begin(), wstrEndToken.end());
  
  m_bFileReadable = ParseFile(strFilename, bStopOnEmptyLine,
                              strToken, strEndToken);
}


KeyValueFileParser::KeyValueFileParser(ifstream& fileData,
                                       bool bStopOnEmptyLine,
                                       const wstring& wstrToken,
                                       const wstring& wstrEndToken)
{
  
  string strToken(wstrToken.begin(), wstrToken.end());
  string strEndToken(wstrEndToken.begin(), wstrEndToken.end());
  
  m_bFileReadable = ParseFile(fileData, bStopOnEmptyLine,
                              strToken, strEndToken);
}

KeyValueFileParser::KeyValueFileParser(ifstream& fileData,
                                       bool bStopOnEmptyLine,
                                       const string& strToken,
                                       const string& strEndToken)
{
  m_bFileReadable = ParseFile(fileData, bStopOnEmptyLine,
                              strToken, strEndToken);
}


KeyValueFileParser::~KeyValueFileParser()
{
}

template<bool touppercase>
struct matching_keys : public std::binary_function<std::string, KeyValPair, bool> {
  bool operator()(const std::string &key, const KeyValPair &kv) const {
    if (touppercase)
      return key == kv.strKeyUpper;
    else
      return key == kv.strKey;
  }
};

template<bool touppercase>
struct wmatching_keys : public std::binary_function<std::wstring, KeyValPair, bool> {
  bool operator()(const std::wstring &key, const KeyValPair &kv) const {
    if (touppercase)
      return key == kv.wstrKeyUpper;
    else
      return key == kv.wstrKey;
  }
};

const KeyValPair* KeyValueFileParser::GetData(const std::string& strKey,
                                              const bool bCaseSensitive) const
{
  const std::vector<const KeyValPair*>& v = KeyValueFileParser::GetDataVec(strKey,bCaseSensitive);
  if(v.empty()) {
    return NULL;
  } else {
    return v[0];
  }
}

const KeyValPair* KeyValueFileParser::GetData(const std::wstring& wstrKey,
                                              const bool bCaseSensitive) const
{
  const std::vector<const KeyValPair*>& v = KeyValueFileParser::GetDataVec(wstrKey,bCaseSensitive);
  if(v.empty()) {
    return NULL;
  } else {
    return v[0];
  }
}


const std::vector<const KeyValPair*> KeyValueFileParser::GetDataVec(
                                                                    const std::string& strKey,
                                                                    const bool bCaseSensitive) const
{
  std::vector<const KeyValPair*> result;
  std::string key(strKey);
  
  if(!bCaseSensitive) {
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
  }
  
  std::vector<KeyValPair>::const_iterator e = m_vecTokens.end();
  for (std::vector<KeyValPair>::const_iterator iter = m_vecTokens.begin();;
       iter++) {
    
    if (bCaseSensitive)
      iter = std::find_if(iter, e, std::bind(matching_keys<false>(), key, std::placeholders::_1));
    else
      iter = std::find_if(iter, e, std::bind(matching_keys<true>(), key, std::placeholders::_1));
    
    if(iter == m_vecTokens.end())
      break;
    else
      result.push_back(&(*iter));
  }
  
  return result;
}

const std::vector<const KeyValPair*> KeyValueFileParser::GetDataVec(
                                                                    const std::wstring& wstrKey,
                                                                    const bool bCaseSensitive) const
{
  std::vector<const KeyValPair*> result;
  std::wstring key(wstrKey);
  
  if(!bCaseSensitive) {
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
  }
  
  std::vector<KeyValPair>::const_iterator e = m_vecTokens.end();
  for (std::vector<KeyValPair>::const_iterator iter = m_vecTokens.begin();;
       iter++) {
    if (bCaseSensitive)
      iter = std::find_if(iter,e,std::bind(wmatching_keys<false>(), key, std::placeholders::_1));
    else
      iter = std::find_if(iter,e,std::bind(wmatching_keys<true>(), key, std::placeholders::_1));
    
    if(iter == m_vecTokens.end())
      break;
    else
      result.push_back(&(*iter));
  }
  
  return result;
}

bool KeyValueFileParser::ParseFile(const std::string& strFilename,
                                   bool bStopOnEmptyLine,
                                   const std::string& strToken,
                                   const std::string& strEndToken) {
  ifstream fileData(strFilename.c_str(),ios::binary);
  bool result = ParseFile(fileData, bStopOnEmptyLine, strToken, strEndToken);
  fileData.close();
  return result;
}

bool KeyValueFileParser::ParseKeyValueLine(std::string line,
                                           bool bStopOnEmptyLine,
                                           bool bStopOnInvalidLine,
                                           const std::string& strToken,
                                           const std::string& strEndToken) {
  line = SysTools::TrimStrLeft(line);
  
  // remove windows line endings
  if (line.length() > 0 && line[line.length()-1] == 13)
    line = line.substr(0,line.length()-1);
  
  if ((strEndToken != "" && strEndToken == line) ||
			(bStopOnEmptyLine && line.empty()))  {
    return false;
  }
  
  // skip comments
  size_t cPos = line.find_first_of('#');
  if (cPos != std::string::npos) line = line.substr(0,cPos);
  line = SysTools::TrimStr(line);
  if (line.length() == 0) return true; // skips empty and comment lines
  
  // skip invalid lines
  if (line.find_first_of(strToken) == string::npos)
    return !bStopOnInvalidLine;
  
  string strKey = SysTools::TrimStrRight(line.substr(0, line.find_first_of(strToken)));
  
  line = SysTools::TrimStr(line.substr(line.find_first_of(strToken)+strToken.length(),
                                       line.length()));
  
  if (strKey.empty()) return true;
  
  KeyValPair newKey(strKey, line);
  m_vecTokens.push_back(newKey);
  
  return true;
}


bool KeyValueFileParser::ParseFile(ifstream& fileData, bool bStopOnEmptyLine,
                                   const std::string& strToken,
                                   const std::string& strEndToken) {
  string line;
  
  m_iStopPos = 0;
  if (fileData.is_open())
  {
		bool bContinue = true;
		while (! fileData.eof() && bContinue )
		{
		  getline (fileData,line);
		  bContinue = ParseKeyValueLine(line, bStopOnEmptyLine, false,
                                    strToken, strEndToken);
      
		  if (!bContinue) m_iStopPos = size_t(fileData.tellg());
		}
  } else return false;
  
  return true;
}

/*
 The MIT License
 
 Copyright (c) 2013 Jens Krueger
 
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

