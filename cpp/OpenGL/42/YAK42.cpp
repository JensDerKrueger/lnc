#include <array>

#include <Tesselation.h>

#include "YAK42.h"

#include "Vec4.h"

const Vec3 brickScale{7.8f,9.6f,7.8f};
constexpr float studHeight{1.8f};
constexpr float studRadius{2.4f};
const float studSpacing = brickScale.x-studRadius*2.0f;


const std::array<Vec4,111> YAK42::colors {{
  {0.95f,0.95f,0.95f,1.0f},
  {0.63f,0.65f,0.64f,1.0f},
  {0.98f,0.91f,0.60f,1.0f},
  {0.84f,0.77f,0.60f,1.0f},
  {0.76f,0.85f,0.72f,1.0f},
  {0.91f,0.73f,0.78f,1.0f},
  {0.80f,0.52f,0.26f,1.0f},
  {0.80f,0.56f,0.41f,1.0f},
  {0.77f,0.16f,0.11f,1.0f},
  {0.77f,0.44f,0.63f,1.0f},
  {0.05f,0.41f,0.67f,1.0f},
  {0.96f,0.80f,0.18f,1.0f},
  {0.38f,0.28f,0.20f,1.0f},
  {0.11f,0.16f,0.20f,1.0f},
  {0.43f,0.43f,0.42f,1.0f},
  {0.16f,0.50f,0.27f,1.0f},
  {0.63f,0.77f,0.55f,1.0f},
  {0.95f,0.81f,0.61f,1.0f},
  {0.29f,0.59f,0.29f,1.0f},
  {0.63f,0.37f,0.20f,1.0f},
  {0.76f,0.79f,0.87f,1.0f},
  {0.71f,0.82f,0.89f,1.0f},
  {0.93f,0.77f,0.71f,1.0f},
  {0.85f,0.53f,0.47f,1.0f},
  {0.43f,0.60f,0.79f,1.0f},
  {0.78f,0.76f,0.72f,1.0f},
  {0.42f,0.20f,0.48f,1.0f},
  {0.89f,0.61f,0.25f,1.0f},
  {0.85f,0.52f,0.25f,1.0f},
  {0.00f,0.56f,0.61f,1.0f},
  {0.41f,0.36f,0.26f,1.0f},
  {0.26f,0.33f,0.58f,1.0f},
  {0.41f,0.45f,0.67f,1.0f},
  {0.78f,0.82f,0.24f,1.0f},
  {0.33f,0.65f,0.69f,1.0f},
  {0.72f,0.84f,0.84f,1.0f},
  {0.64f,0.74f,0.27f,1.0f},
  {0.85f,0.89f,0.65f,1.0f},
  {0.91f,0.67f,0.35f,1.0f},
  {0.83f,0.44f,0.30f,1.0f},
  {0.57f,0.22f,0.47f,1.0f},
  {0.92f,0.72f,0.57f,1.0f},
  {0.86f,0.74f,0.51f,1.0f},
  {0.68f,0.48f,0.35f,1.0f},
  {0.61f,0.64f,0.66f,1.0f},
  {0.45f,0.53f,0.61f,1.0f},
  {0.53f,0.49f,0.56f,1.0f},
  {0.88f,0.60f,0.39f,1.0f},
  {0.58f,0.54f,0.45f,1.0f},
  {0.13f,0.23f,0.34f,1.0f},
  {0.15f,0.27f,0.17f,1.0f},
  {0.47f,0.53f,0.63f,1.0f},
  {0.58f,0.56f,0.64f,1.0f},
  {0.58f,0.53f,0.40f,1.0f},
  {0.34f,0.35f,0.34f,1.0f},
  {0.09f,0.11f,0.20f,1.0f},
  {0.67f,0.68f,0.67f,1.0f},
  {0.47f,0.56f,0.51f,1.0f},
  {0.58f,0.47f,0.46f,1.0f},
  {0.48f,0.18f,0.18f,1.0f},
  {0.46f,0.42f,0.38f,1.0f},
  {0.84f,0.66f,0.29f,1.0f},
  {0.51f,0.54f,0.36f,1.0f},
  {0.98f,0.84f,0.18f,1.0f},
  {0.91f,0.67f,0.18f,1.0f},
  {0.41f,0.25f,0.15f,1.0f},
  {0.81f,0.38f,0.14f,1.0f},
  {0.64f,0.64f,0.64f,1.0f},
  {0.27f,0.40f,0.64f,1.0f},
  {0.14f,0.28f,0.55f,1.0f},
  {0.56f,0.26f,0.52f,1.0f},
  {0.39f,0.37f,0.38f,1.0f},
  {0.90f,0.89f,0.87f,1.0f},
  {0.69f,0.56f,0.27f,1.0f},
  {0.44f,0.58f,0.47f,1.0f},
  {0.47f,0.71f,0.71f,1.0f},
  {0.62f,0.76f,0.91f,1.0f},
  {0.42f,0.51f,0.72f,1.0f},
  {0.56f,0.30f,0.16f,1.0f},
  {0.49f,0.36f,0.27f,1.0f},
  {0.59f,0.44f,0.62f,1.0f},
  {0.42f,0.38f,0.61f,1.0f},
  {0.65f,0.66f,0.81f,1.0f},
  {0.80f,0.38f,0.60f,1.0f},
  {0.89f,0.68f,0.78f,1.0f},
  {0.86f,0.56f,0.58f,1.0f},
  {0.94f,0.84f,0.63f,1.0f},
  {0.92f,0.72f,0.50f,1.0f},
  {0.99f,0.92f,0.55f,1.0f},
  {0.49f,0.73f,0.87f,1.0f},
  {0.20f,0.17f,0.46f,1.0f},
  {0.93f,0.93f,0.93f,1.0f},
  {0.80f,0.33f,0.29f,1.0f},
  {0.76f,0.87f,0.94f,1.0f},
  {0.48f,0.71f,0.91f,1.0f},
  {0.97f,0.95f,0.55f,1.0f},
  {0.85f,0.52f,0.42f,1.0f},
  {0.52f,0.71f,0.55f,1.0f},
  {0.97f,0.95f,0.52f,1.0f},
  {0.93f,0.91f,0.87f,1.0f},
  {0.75f,0.72f,0.69f,1.0f},
  {0.89f,0.68f,0.78f,1.0f},
  {0.65f,0.65f,0.80f,1.0f},
  {0.84f,0.45f,0.24f,1.0f},
  {0.85f,0.87f,0.34f,1.0f},
  {0.81f,0.89f,0.97f,1.0f},
  {1.00f,0.96f,0.48f,1.0f},
  {0.88f,0.64f,0.76f,1.0f},
  {0.59f,0.41f,0.36f,1.0f},
  {0.71f,0.52f,0.33f,1.0f},
  {0.54f,0.53f,0.53f,1.0f},
}};


SimpleYAK42::SimpleYAK42(uint16_t width, uint16_t depth, uint16_t height,
                         uint16_t colorCode, const Vec3i& pos) :
YAK42(pos),
width(width),
depth(depth),
height(height),
colorCode(colorCode)
{
  generateGeometry();
}

void SimpleYAK42::render(GLApp& app) const  {
  app.drawTriangles(geometry, TrisDrawType::LIST, false, true);
}
  
std::vector<Vec2t<uint16_t>> SimpleYAK42::studsTop() {
  return studsBottom();
}

std::vector<Vec2t<uint16_t>> SimpleYAK42::studsBottom() {
  std::vector<Vec2t<uint16_t>> studs(depth*width);
  size_t i = 0;
  for (uint16_t y = 0; y < depth; ++y) {
    for (uint16_t x = 0; x < width; ++x) {
      studs[i].x = x;
      studs[i++].y = y;
    }
  }
  return studs;
}

void SimpleYAK42::generateGeometry() {
  geometry.clear();
  generateStuds();
  generateBase();
}

void SimpleYAK42::generateStuds() {
  
  for (uint16_t z = 0; z < depth;++z) {
    for (uint16_t x = 0; x < width;++x) {
      
      const Vec3 relativePos{
        -0.5f*brickScale.x*width+studSpacing/2.0f+studRadius + (studSpacing+2*studRadius)*x,
        height/3.0f*brickScale.y/2.0f+studHeight*0.5f,
        -0.5f*brickScale.z*depth+studSpacing/2.0f+studRadius + (studSpacing+2*studRadius)*z
      };
      
      Tesselation studTesselation = Tesselation::genCylinder(Vec3(getPos())*brickScale+relativePos,
                                                             studRadius,
                                                             studHeight,
                                                             false,
                                                             true, 20);
      
      const std::vector<float> vertices   = studTesselation.getVertices();
      const std::vector<float> normals    = studTesselation.getNormals();
      const std::vector<float> texcoords  = studTesselation.getTexCoords();
      const std::vector<uint32_t> indices = studTesselation.getIndices();
              
      for (const uint32_t index : indices) {
        geometry.push_back(vertices[index*3+0]);
        geometry.push_back(vertices[index*3+1]);
        geometry.push_back(vertices[index*3+2]);

        geometry.push_back(colors[colorCode].r);
        geometry.push_back(colors[colorCode].g);
        geometry.push_back(colors[colorCode].b);
        geometry.push_back(colors[colorCode].a);        

        geometry.push_back(normals[index*3+0]);
        geometry.push_back(normals[index*3+1]);
        geometry.push_back(normals[index*3+2]);
      }
    }
  }

}

void SimpleYAK42::generateBase() {
  Tesselation baseTesselation = Tesselation::genBrick(Vec3(getPos())*brickScale,
                                                      Vec3(width,height/3.0f,depth)*brickScale);

  
  const std::vector<float> vertices   = baseTesselation.getVertices();
  const std::vector<float> normals    = baseTesselation.getNormals();
  const std::vector<uint32_t> indices = baseTesselation.getIndices();
      
  for (const uint32_t index : indices) {
    geometry.push_back(vertices[index*3+0]);
    geometry.push_back(vertices[index*3+1]);
    geometry.push_back(vertices[index*3+2]);

    geometry.push_back(colors[colorCode].r);
    geometry.push_back(colors[colorCode].g);
    geometry.push_back(colors[colorCode].b);
    geometry.push_back(colors[colorCode].a);

    geometry.push_back(normals[index*3+0]);
    geometry.push_back(normals[index*3+1]);
    geometry.push_back(normals[index*3+2]);
  }
  
}
