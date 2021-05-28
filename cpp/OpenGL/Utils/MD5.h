#pragma once

#include <vector>
#include <string>
#include <stdint.h>

typedef std::vector<uint8_t> MD5Hash;

class MD5 {
public:
  MD5();
  virtual ~MD5() {}

  void transform(uint8_t Block[64], int& error);
  void update(uint8_t* Input, uint32_t nInputLen, int& error);
  MD5Hash final(int& iErrorCalculate);

  static std::string toHexString(const MD5Hash & data);
  static MD5Hash toHash(const std::string& str);
  static bool verifyMD5(const std::string& filename, const MD5Hash& testHash, size_t iOffset = 0);
  static MD5Hash computeMD5(const std::string& filename, size_t iOffset = 0);
  static MD5Hash computeMD5(const std::vector<uint8_t>& data);

private:
  inline uint32_t rotateLeft(uint32_t x, int n);
  inline void FF( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);
  inline void GG( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);
  inline void HH( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);
  inline void II( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);

  inline void uintToChar(uint8_t* Output, uint32_t* Input, uint32_t nLength, int& error);
  inline void charToUint(uint32_t* Output, uint8_t* Input, uint32_t nLength, int& error);
  void memoryMove(uint8_t* from, uint8_t* to, uint32_t size);

  uint8_t  m_lpszBuffer[64];    // InputBuffer
  uint32_t m_nCount[2] ;        // bitcount, modulo 2^64 (lsb first)
  uint32_t m_lMD5[4] ;          // MD5 sum
};
