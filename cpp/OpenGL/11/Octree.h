#pragma once

#include <vector>
#include <array>
#include <memory>

#include <Vec3.h>


class OctreeNode;

typedef std::shared_ptr<OctreeNode> OctreeNodePtr;

class OctreeNode {
public:
    OctreeNode(const Vec3& minPos, const Vec3& maxPos);
    OctreeNode(const Vec3& first, const Vec3& minPos, const Vec3& maxPos);
    void add(const Vec3& pos, size_t maxElemCount);

private:
    Vec3 minPos;
    Vec3 maxPos;
    std::vector<Vec3> elements;
    std::array<OctreeNodePtr, 8> children;

    void split(size_t maxElemCount);
    
};

class Octree {
public:
    Octree(float size, const Vec3& first, size_t maxElemCount=10, size_t maxDepth=100);
    
    void add(const Vec3& pos);
    float minDist(const Vec3& pos) const;
    
private:
    float size;
    size_t maxElemCount;
    size_t maxDepth;
    OctreeNodePtr root;
};
