#pragma once

#include "Base64.h"

class AESCrypt {
public:
  AESCrypt(const uint8_t* iv,
           const uint8_t* key);
  AESCrypt(const uint8_t* iv,
           const std::string& key);
  AESCrypt(const std::string& iv,
           const std::string& key);
  
  std::string encryptString(const std::string& plainText);
  std::string decryptString(const std::string& cipher);
  
  void encrypt(const std::vector<uint8_t>& plain, std::vector<uint8_t>& cipher);
  bool decrypt(const std::vector<uint8_t>& cipher, std::vector<uint8_t>& plain);
  
  static uint32_t getKeyLength();
  
  static void genIV(uint8_t iv[16]);
  
private:
  // state-array holding the intermediate results during decryption.
  typedef uint8_t state_t[4][4];
  state_t* state;
  
  // The array that stores the round keys.
  uint8_t RoundKey[176];
  
  // Initial Vector
  uint8_t Iv[16];
  
  void encrypt_buffer(uint8_t* output, uint8_t* input, size_t length);
  void decrypt_buffer(uint8_t* output, const uint8_t* input, size_t length);
  
  void KeyExpansion(const uint8_t* Key);
  void AddRoundKey(uint8_t round);
  void SubBytes(void);
  void ShiftRows(void);
  void MixColumns(void);
  void InvMixColumns(void);
  void InvSubBytes(void);
  void InvShiftRows(void);
  void Cipher(void);
  void InvCipher(void);
  void XorWithIv(uint8_t* buf);
  
};

/*
 The MIT License
 
 Copyright (c) 2016 Jens Krueger
 (based on "Tiny AES128 in C")
 
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

