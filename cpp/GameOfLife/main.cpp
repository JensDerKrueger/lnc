#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>

class Grid2D {
public:
  Grid2D(size_t width, size_t height) :
    width(width),
    height(height),
    data(this->width*this->height) {}
  
  void setData(size_t x, size_t y, bool value) {
    data[index(x,y)] = value;
  }
  
  bool getData(size_t x, size_t y) const {
    return data[index(x,y)];
  }
  
  size_t getHeight() const {return height;}
  size_t getWidth() const {return width;}
  
  
  size_t countNeighbours(size_t x, size_t y) const {
    size_t count = 0;
    for (int64_t dy = -1;dy<=1;++dy) {
      for (int64_t dx = -1;dx<=1;++dx) {
        if (dx==0 && dy == 0) continue;
        count += getDataCyclic(int64_t(x)+dx, int64_t(y)+dy) ? 1 : 0;
      }
    }
    return count;
  }
  
private:
  size_t width;
  size_t height;
  std::vector<bool> data;
  
  size_t index(size_t x, size_t y) const {
    return y*width+x;
  }
  
  bool getDataCyclic(int64_t x, int64_t y) const {
    x = (x+width) % width;
    y = (y+height) % height;
    return getData(x,y);
  }
  
};

void init(Grid2D& g) {
  /*
  g.setData( 9,10, true);
  g.setData(10,10, true);
  g.setData(11,10, true);
  g.setData(11, 9, true);
  g.setData(10, 8, true);
  */
  g.setData(10,10, true);
  g.setData(10,11, true);
  g.setData(10, 9, true);
  g.setData( 9,10, true);
  g.setData(11, 9, true);
}


void setColor(float r, float g, float b) {
  const uint32_t index = 16 + uint32_t(r*5) +
                          6 * uint32_t(g*5) +
                         36 * uint32_t(b*5);
  std::cout << "\033[48;5;" << index << "m";
}

void clear() {
  std::cout << "\033[2J\033[;H";
}

void render(const Grid2D& g) {
  clear();
  for (size_t y = 0;y<g.getHeight();++y) {
    for (size_t x = 0;x<g.getWidth();++x) {
      setColor(g.getData(x,y) ? 1.0f : 0.0f, 0.0f, 0.0f);
      std::cout << "  " ;
    }
    std::cout << "\n";
  }
  setColor(0.0f, 0.0f, 0.0f);
}

void play(const Grid2D& currentGrid, Grid2D& nextGrid) {
  for (size_t y = 0;y<currentGrid.getHeight();++y) {
    for (size_t x = 0;x<currentGrid.getWidth();++x) {
      const size_t n{currentGrid.countNeighbours(x,y)};
      const bool currentCell{currentGrid.getData(x,y)};
      
      if (currentCell)
        nextGrid.setData(x,y, n == 2 || n == 3);
      else
        nextGrid.setData(x,y, n == 3);
    }
  }
}

int main(int argc, char** argv) {
  const size_t width{30};
  const size_t height{30};
    
  Grid2D currentGrid{width,height};
  Grid2D nextGrid{width,height};
  
  init(currentGrid);
    
  while (true) {
    render(currentGrid);
    play(currentGrid, nextGrid);
    std::swap(currentGrid, nextGrid);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
    
  return EXIT_SUCCESS;
}
