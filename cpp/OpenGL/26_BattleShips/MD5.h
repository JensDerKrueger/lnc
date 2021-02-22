#pragma once

#include <array>
#include <vector>
#include <cstdint>

class MD5 {
public:
  MD5();
  virtual ~MD5() {};
  
  void transform(const uint8_t Block[64], int& error);
  void update(const uint8_t* Input, uint32_t nInputLen, int& error);
  std::vector<uint8_t> final(int& iErrorCalculate);
  
protected:
  inline uint32_t rotateLeft(uint32_t x, int n) const;
  inline void FF( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);
  inline void GG( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);
  inline void HH( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);
  inline void II( uint32_t& A, uint32_t B, uint32_t C, uint32_t D, uint32_t X, uint32_t S, uint32_t T);
  
  inline void uIntToByte(uint8_t* Output, const uint32_t* Input, uint32_t nLength, int& error);
  inline void byteToUInt(uint32_t* Output, const uint8_t* Input, uint32_t nLength, int& error);
  void memoryMove(uint8_t* from, uint8_t* to, uint32_t size);
  
private:
  uint8_t  m_lpszBuffer[64];    // InputBuffer
  uint32_t m_nCount[2] ;        // bitcount, modulo 2^64 (lsb first)
  uint32_t m_lMD5[4] ;          // MD5 sum
};

template<typename Iter, typename T>
std::array<uint8_t,16> md5(Iter begin, Iter end) {
  MD5 md;
  int error = 0;
  while(begin != end) {
    md.update((const uint8_t*)&*begin, sizeof(T), error);
    ++begin;
  }
  std::vector<uint8_t> final = md.final(error);
  
  std::array<uint8_t,16> rv;
  std::copy(final.begin(), final.end(), rv.begin());
  return rv;
}
