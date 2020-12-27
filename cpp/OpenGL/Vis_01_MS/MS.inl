#include <array>

/*
  0__________0________1
  |                   |
  |                   |
  |                   |
  |3                  |1
  |                   |
  |                   |
  |                   |
  3_________2_________2
*/

static std::array<uint16_t,16> edgeTable = {
  0b0000, 0b1001, 0b0011, 0b1010, 0b0110, 0b1111, 0b0101, 0b1100,
  0b1100, 0b0101, 0b1111, 0b0110, 0b1010, 0b0011, 0b1001, 0b0000
};

static std::array<std::array<uint8_t,2>,4> edgeToVertexTable = {{
  {0,1},
  {1,2},
  {2,3},
  {3,0}
}};

static std::array<Vec2,4> vertexPosTable = {
  Vec2{0,1},
  Vec2{1,1},
  Vec2{1,0},
  Vec2{0,0}
};
