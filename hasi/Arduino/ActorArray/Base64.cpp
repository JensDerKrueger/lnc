#include "Base64.h"

#ifdef ARDUINO
  #define STR_LENGTH length
  #define STR_FIND indexOf
#else
  #define STR_LENGTH size 
  #define STR_FIND find
#endif


static const B64_PORTABLE_STRING base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(uint8_t c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

B64_PORTABLE_STRING base64_encode(uint8_t const* buf, unsigned int bufLen) {
  B64_PORTABLE_STRING ret;
  int i = 0;
  int j = 0;
  uint8_t char_array_3[3];
  uint8_t char_array_4[4];
  
  while (bufLen--) {
    char_array_3[i++] = *(buf++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      
      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }
  
  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';
    
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;
    
    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];
    
    while((i++ < 3))
      ret += '=';
  }
  
  return ret;
}

void base64_decode(const B64_PORTABLE_STRING& encoded_string, SimpleVec& result) {
  int in_len = (int)encoded_string.STR_LENGTH();
  int i = 0;
  int j = 0;
  int in_ = 0;
  uint8_t char_array_4[4], char_array_3[3];
  
  result.clear();
  
  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = (unsigned char)base64_chars.STR_FIND(char_array_4[i]);
      
      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
      
      for (i = 0; (i < 3); i++)
        result.append(char_array_3[i]);
      i = 0;
    }
  }
  
  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;
    
    for (j = 0; j <4; j++)
      char_array_4[j] = (unsigned char)base64_chars.STR_FIND(char_array_4[j]);
    
    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
    
    for (j = 0; (j < i - 1); j++)
      result.append(char_array_3[j]);
  }
}


SimpleVec::SimpleVec() :
_length(0),
_data(0),
_capacity(0)
{
}

SimpleVec::SimpleVec(uint32_t length) :
_length(0),
_data(0),
_capacity(0)
{
  setLength(length);
}

SimpleVec::SimpleVec(const B64_PORTABLE_STRING& s) :
_length(0),
_data(0),
_capacity(0)
{
  setLength(uint32_t(s.length()));
  for (uint32_t i = 0;i<_length;++i)
    _data[i] = s[i];
}

SimpleVec::SimpleVec(uint8_t* data, uint32_t dataLength) :
_length(0),
_data(0),
_capacity(0)
{
  setLength(dataLength);
  memcpy(_data, data, dataLength);
}

SimpleVec::SimpleVec(const SimpleVec& other) :
_length(0),
_data(0),
_capacity(0)
{
  setLength(other._length);
  memcpy(_data, other._data, other._length);
}

SimpleVec::SimpleVec(const SimpleVec& other, uint32_t newLength, uint8_t fill) :
_length(0),
_data(0),
_capacity(0)
{
  setLength(newLength);
  
  if (newLength > other.length()) {
    memset(_data, fill, newLength);
    memcpy(_data, other._data, other._length);
  } else {
    memcpy(_data, other._data, newLength);
  }
}

SimpleVec& SimpleVec::operator=(const SimpleVec &other) {
  setLength(other._length);
  if (other._length > 0)
    memcpy(_data, other._data, other._length);
  return *this;
}

SimpleVec::~SimpleVec() {
  delete [] _data;
}

void SimpleVec::setLength(uint32_t newLength) {
  if (newLength > _capacity) {
    if (_data) delete [] _data;
    _capacity = newLength;
    _data = new uint8_t[newLength];
  }
  _length = newLength;
}

void SimpleVec::truncate(uint32_t newLength) {
  if (newLength < _length)
    _length = newLength;
}

void SimpleVec::clear() {
  _length = 0;
}

void SimpleVec::append(uint8_t element) {
  
  if (_length+1 > _capacity) {
    _capacity = _length*2+1;
    uint8_t* newData = new uint8_t[_capacity];
    if (_length > 0) {
      memcpy(newData, _data, _length);
    }
    delete [] _data;
    _data = newData;
  }
  _data[_length] = element;
  _length++;
}

void SimpleVec::remove(uint8_t element) {
  if (_length == 0) return;
  
  uint32_t count = 0;
  for (uint32_t pos = 0;pos < _length;++pos) {
    if (element == _data[pos]) count++;
  }
  if (count == 0) return;
  
  uint32_t targetPos = 0;
  for (uint32_t pos = 0;pos < _length;++pos) {
    if (element != _data[pos]) {
      _data[targetPos++] = _data[pos];
    }
  }
  _length = _length-count;
}

uint32_t SimpleVec::length() const {
  return _length;
}

const uint8_t* SimpleVec::constData() const {
  return _data;
}

uint8_t* SimpleVec::data() {
  return _data;
}

void SimpleVec::subData(uint32_t start, uint32_t len) {
  if (start >= _length) return;
  if (start+len >= _length) len = _length-start;
  
  _length = len;
  
  if (start > 0) {
    for (uint32_t i = start;i<start+len;++i) {
      _data[i-start] = _data[i];
    }
  }
}

B64_PORTABLE_STRING SimpleVec::toString() const {
  B64_PORTABLE_STRING s;
  for (uint32_t i = 0;i<_length;++i)
    s += _data[i];
  return s;
}

/*
 The MIT License
 
 Copyright (c) 2016 Jens Krueger
 (based base64.cpp and base64.h by René Nyffenegger)
 
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
 
 Copyright notive from base64.h:
 
 Copyright (C) 2004-2008 René Nyffenegger
 
 This source code is provided 'as-is', without any express or implied
 warranty. In no event will the author be held liable for any damages
 arising from the use of this software.
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
 1. The origin of this source code must not be misrepresented; you must not
 claim that you wrote the original source code. If you use this source code
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original source code.
 
 3. This notice may not be removed or altered from any source distribution.
 
 René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/