#pragma once

#include <string>
#include <vector>

class Image {
  public:
    Image(unsigned int w, unsigned int h) :
      w(w),
      h(h),
      target(w*h)
    {
    }

    bool save(const std::string& filename, bool ignoreSize=false);

  protected:
    unsigned int w;
    unsigned int h;
  
    void setData(unsigned int x, unsigned int y, char value);
    char getData(unsigned int x, unsigned int y) const;
    std::vector<char>& getDataVector() {return target;}

  private:
    std::vector<char> target;
    std::string getExt(const std::string& filename) const;
    std::string toString() const;

};
