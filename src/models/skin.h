#pragma once

#include "node.h"

struct NodeBindMatrix
{
    Node *node;
    glm::mat4 bindMatrix;
};

struct Skin
{
    std::string name;
    std::vector<NodeBindMatrix> nodeBindMatrices;
    explicit Skin(std::string &name) : name(name) {}

    friend class GLTFLoader;
};

