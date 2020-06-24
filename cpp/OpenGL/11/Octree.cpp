#include <iostream>

#include "Octree.h"


OctreeNode::OctreeNode(const Vec3& first, const Vec3& minPos, const Vec3& maxPos) :
    minPos{minPos},
    maxPos{maxPos},
    elements{first},
    children{nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr}
{
}

OctreeNode::OctreeNode(const Vec3& minPos, const Vec3& maxPos) :
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

void OctreeNode::add(const Vec3& pos, size_t maxElemCount) {
    if (isLeaf()) {
        elements.push_back(pos);
        if (elements.size() >= maxElemCount) {
            split(maxElemCount);
        }
    } else {
        size_t index = subtreeIndex(pos);
        children[index]->add(pos, maxElemCount);
    }
}

void OctreeNode::split(size_t maxElemCount) {
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
        children[index]->add(elements[i], maxElemCount);
    }
    
    elements.clear();
}

std::vector<float> OctreeNode::toTriList() const {
    std::vector<float> result;
        
    if (!isLeaf()) {
        const Vec3 center{computeCenter()};
        
        // tris 0
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(center.z());
        result.push_back(maxPos.x()); result.push_back(minPos.y()); result.push_back(center.z());
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(center.z());
        
        // tris 1
        result.push_back(minPos.x()); result.push_back(minPos.y()); result.push_back(center.z());
        result.push_back(maxPos.x()); result.push_back(maxPos.y()); result.push_back(center.z());
        result.push_back(minPos.x()); result.push_back(maxPos.y()); result.push_back(center.z());

        // tris 2
        result.push_back(minPos.x()); result.push_back(center.y()); result.push_back(minPos.z());
        result.push_back(maxPos.x()); result.push_back(center.y()); result.push_back(minPos.z());
        result.push_back(maxPos.x()); result.push_back(center.y()); result.push_back(maxPos.z());
        
        // tris 3
        result.push_back(minPos.x()); result.push_back(center.y()); result.push_back(minPos.z());
        result.push_back(maxPos.x()); result.push_back(center.y()); result.push_back(maxPos.z());
        result.push_back(minPos.x()); result.push_back(center.y()); result.push_back(maxPos.z());

        // tris 4
        result.push_back(center.x()); result.push_back(minPos.y()); result.push_back(minPos.z());
        result.push_back(center.x()); result.push_back(maxPos.y()); result.push_back(minPos.z());
        result.push_back(center.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z());
        
        // tris 5
        result.push_back(center.x()); result.push_back(minPos.y()); result.push_back(minPos.z());
        result.push_back(center.x()); result.push_back(maxPos.y()); result.push_back(maxPos.z());
        result.push_back(center.x()); result.push_back(minPos.y()); result.push_back(maxPos.z());
        
        for (size_t i = 0;i<children.size();++i) {
            std::vector<float> b = children[i]->toTriList();
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
    root->add(pos, maxElemCount);
}

float Octree::minDist(const Vec3& pos) const {
    return 0.0f;
}

std::vector<float> Octree::toTriList() const {
    return root->toTriList();
}
