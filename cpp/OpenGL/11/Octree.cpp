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


void OctreeNode::add(const Vec3& pos, size_t maxElemCount) {
    elements.push_back(pos);
    if (elements.size() >= maxElemCount) {
        split(maxElemCount);
    }
}

void OctreeNode::split(size_t maxElemCount) {
    const Vec3 center{maxPos/2.0f+minPos/2.0f};
    
    // create childen
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
        size_t index =  ((elements[i].z() <= center.z()) ? 0 : 4) +
                        ((elements[i].y() <= center.y()) ? 0 : 2) +
                        ((elements[i].x() <= center.x()) ? 0 : 1);
        
        children[index]->add(elements[i], maxElemCount);
    }
    
    elements.clear();
}

Octree::Octree(float size, const Vec3& first, size_t maxElemCount, size_t maxDepth) :
    size(size),
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

