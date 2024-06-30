#pragma once

#include "node.h"

struct NodeBindMatrix
{
    Node *node;
    glm::mat4 inverseBindMatrix;
};

struct Skin
{
    std::string name;
    std::vector<NodeBindMatrix> nodeBindMatrices;
    explicit Skin(std::string &name) : name(name) {}
    uPtr<Buffer> jointsBuffer;
    friend class GLTFLoader;
};

