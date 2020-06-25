#include <iostream>
#include <limits>
#include <cmath>

#include "Octree.h"


OctreeNode::OctreeNode(const Vec3& first, const Vec3& minPos, const Vec3& maxPos) :
    warranty{10},
    minPos{minPos},
    maxPos{maxPos},
    elements{first},
    children{nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr}
{
}

OctreeNode::OctreeNode(const Vec3& minPos, const Vec3& maxPos) :
    warranty{10},
    minPos{minPos},
    maxPos{maxPos},
    elements{},
    children{nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr}
{
}

Vec3 OctreeNode::computeCenter() const {
    return maxPos/2.0f+minPos/2.0f;
}

size_t OctreeNode::subtreeIndex(const Vec3& pos) const {
    const Vec3 center{computeCenter()};
    return ((pos.z() <= center.z()) ? 0 : 4) +
           ((pos.y() <= center.y()) ? 0 : 2) +
           ((pos.x() <= center.x()) ? 0 : 1);
}

void OctreeNode::add(const Vec3& pos, size_t maxElemCount, size_t maxDepth) {
    if (isLeaf()) {
        elements.push_back(pos);
        if (maxDepth > 0 && elements.size() > maxElemCount) {
            split(maxElemCount, maxDepth);
        }
    } else {
        size_t index = subtreeIndex(pos);
        children[index]->add(pos, maxElemCount, maxDepth-1);
    }
}

void OctreeNode::split(size_t maxElemCount, size_t maxDepth) {
    // create childen
    const Vec3 center{computeCenter()};
    children[0] = std::make_shared<OctreeNode>(minPos,center);
    children[1] = std::make_shared<OctreeNode>(Vec3(center.x(),minPos.y(),minPos.z()),
                                               Vec3(maxPos.x(),center.y(),center.z()));
    children[2] = std::make_shared<OctreeNode>(Vec3(minPos.x(),center.y(),minPos.z()),
                                               Vec3(center.x(),maxPos.y(),center.z()));
    children[3] = std::make_shared<OctreeNode>(Vec3(center.x(),center.y(),minPos.z()),
                                               Vec3(maxPos.x(),maxPos.y(),center.z()));

    children[4] = std::make_shared<OctreeNode>(Vec3(minPos.x(),minPos.y(),center.z()),
                                               Vec3(center.x(),center.y(),maxPos.z()));
    children[5] = std::make_shared<OctreeNode>(Vec3(center.x(),minPos.y(),center.z()),
                                               Vec3(maxPos.x(),center.y(),maxPos.z()));
    children[6] = std::make_shared<OctreeNode>(Vec3(minPos.x(),center.y(),center.z()),
                                               Vec3(center.x(),maxPos.y(),maxPos.z()));
    children[7] = std::make_shared<OctreeNode>(center, maxPos);

    // insert elements into children
    for (size_t i = 0;i<elements.size();++i) {
        size_t index = subtreeIndex(elements[i]);
        children[index]->add(elements[i], maxElemCount, maxDepth-1);
    }
    
    elements.clear();
}

float OctreeNode::minSqDistApprox(const Vec3& pos) const {
    float minSqDistance = std::numeric_limits<float>::max();
    if (isLeaf()) {
        for (const Vec3& e : elements) {
            minSqDistance = std::min(minSqDistance, (pos-e).sqlength());
        }
        return minSqDistance;
    } else {
        size_t index = subtreeIndex(pos);
        if (children[index]->isLeaf() && children[index]->elements.size() == 0) {
            for (size_t i = 0;i<8;++i) {
                minSqDistance = std::min(minSqDistance, children[i]->minSqDistApprox(pos));
            }
            return minSqDistance;
        } else {
            return children[index]->minSqDistApprox(pos);
        }
    }
}

bool OctreeNode::intersect(const Vec3& pos, float radiusSq, Vec3 minPos, Vec3 maxPos) const {
    if (pos.x() < minPos.x()) {
        radiusSq -= sq(minPos.x() - pos.x());
    } else if (pos.x() > maxPos.x()) {
        radiusSq -= sq(maxPos.x() - pos.x());
    }
    if (pos.y() < minPos.y()) {
        radiusSq -= sq(minPos.y() - pos.y());
    } else if (pos.y() > maxPos.y()) {
        radiusSq -= sq(maxPos.y() - pos.y());
    }
    if (pos.z() < minPos.z()) {
        radiusSq -= sq(minPos.z() - pos.z());
    } else if (pos.z() > maxPos.z()) {
        radiusSq -= sq(maxPos.z() - pos.z());
    }
    return radiusSq > 0;
}

float OctreeNode::minSqDist(const Vec3& pos, float radiusSq) const {
    if (isLeaf()) {
        return std::min(radiusSq, minSqDistApprox(pos));
    } else {
        for (size_t i = 0;i<8;++i) {
            if (intersect(pos, radiusSq, children[i]->minPos, children[i]->maxPos))
                radiusSq = std::min(radiusSq, children[i]->minSqDist(pos, radiusSq));
        }
    }
    return radiusSq;
}

void OctreeNode::pushColor(std::vector<float>& v, const Vec4& c) {
    v.push_back(c.r());
    v.push_back(c.g());
    v.push_back(c.b());
    v.push_back(c.a());
}

std::vector<float> OctreeNode::toTriList() {
    std::vector<float> result;
    
    age();
    
    Vec4 color{(warranty == 0) ? Vec4{0.01,0.01,0.01,0.01} : Vec4{0.01,0.0,0.0,0.01}};
        
    if (!isLeaf()) {
        const Vec3 center{computeCenter()};
        
        // tris 0
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(center.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(center.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(center.z()); pushColor(result,color);
        
        // tris 1
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(center.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(center.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(center.z()); pushColor(result,color);

        // tris 2
        result.push_back(minPos.x()); result.push_back(center.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(center.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(center.y()); result.push_back(maxPos.z()); pushColor(result,color);
        
        // tris 3
        result.push_back(minPos.x()); result.push_back(center.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(center.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(center.y()); result.push_back(maxPos.z()); pushColor(result,color);

        // tris 4
        result.push_back(center.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(center.x()); result.push_back(maxPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(center.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        
        // tris 5
        result.push_back(center.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(center.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(center.x()); result.push_back(minPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        
        for (size_t i = 0;i<children.size();++i) {
            std::vector<float> b = children[i]->toTriList();
            result.insert(result.end(), b.begin(), b.end());
        }
    }
    return result;
}


std::vector<float> OctreeNode::toLineList() const {
    std::vector<float> result;
        
    Vec4 color{(warranty == 0) ? Vec4{1.0,1.0,1.0,1.0} : Vec4{1.0,0.0,0.0,1.0}};
    
    if (!isLeaf()) {
        // front back
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(minPos.z()); pushColor(result,color);

        // back quad
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        
        // connections
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(maxPos.z()); pushColor(result,color);

        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);

        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(maxPos.z()); pushColor(result,color);

        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(minPos.z()); pushColor(result,color);
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z()); pushColor(result,color);
        
        for (size_t i = 0;i<children.size();++i) {
            std::vector<float> b = children[i]->toLineList();
            result.insert(result.end(), b.begin(), b.end());
        }
    }
    return result;
}

Octree::Octree(float size, const Vec3& first, size_t maxElemCount, size_t maxDepth) :
    maxElemCount(maxElemCount),
    maxDepth(maxDepth),
    root{std::make_shared<OctreeNode>(first, Vec3(-size,-size,-size),Vec3(size,size,size))}
{
}

void Octree::add(const Vec3& pos) {
    root->add(pos, maxElemCount, maxDepth);
}

float Octree::minDist(const Vec3& pos) const {
    if (root->isLeaf()) {
        return sqrt(root->minSqDistApprox(pos));
    }
    float minDist = root->minSqDistApprox(pos);
    return sqrt(root->minSqDist(pos, minDist));
}

std::vector<float> Octree::toTriList() {
    return root->toTriList();
}

std::vector<float> Octree::toLineList() const {
    return root->toLineList();
}
