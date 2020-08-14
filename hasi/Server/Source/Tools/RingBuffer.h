#pragma once

#include <vector>
#include <iostream>

template <class T>
class RingBuffer {
public:
  RingBuffer(size_t reserved) :
    _size(0),
    _data(reserved),
    _head(0),
    _tail(0)
  {
    _data.resize(reserved);
  }
  
  size_t size() const {
    return _size;
  }
  
  size_t reserved() const {
    return _data.size();
  }
  
  void put(const T& elem) {
    _data[_tail] = elem;
    if (_head == _tail) _head=inc(_head);
    _tail = inc(_tail);
    
    if (_size < _data.size()) _size++;
  }
  
  const T* get(size_t pos=0) {
    if (pos>=_size)
      return nullptr;
    else
      return &(_data[inc(_head,pos)]);
  }
  
  
private:
  size_t _size;
  std::vector<T> _data;
  size_t _head;
  size_t _tail;
  
  size_t inc(size_t& v, size_t i=1) {
    return (v + i) % _data.size();
  }
  
};

/*
 The MIT License
 
 Copyright (c) 2015 CoViDAG.
 
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