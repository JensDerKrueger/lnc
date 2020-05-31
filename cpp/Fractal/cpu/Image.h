#pragma once

#include <string>
#include <vector>

class Image {
  public:
    Image(uint32_t w, uint32_t h) :
      w(w),
      h(h),
      target(w*h)
    {
    }

    bool save(const std::string& filename);

  protected:
    uint32_t w;
    uint32_t h;
  
    void setData(uint32_t x, uint32_t y, uint8_t value);
    uint8_t getData(uint32_t x, uint32_t y) const;
    std::vector<uint8_t>& getDataVector() {return target;}

  private:
    std::vector<uint8_t> target;
    std::string getExt(const std::string& filename) const;
    std::string toString() const;

};
