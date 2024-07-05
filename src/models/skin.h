#pragma once

class Node;

struct NodeBindMatrix
{
    Node *node;
    glm::mat4 inverseBindMatrix;
};

struct Skin
{
    std::string name;
    std::vector<NodeBindMatrix> nodeBindMatrices;
    std::vector<uPtr<Buffer>> jointBuffers;
    explicit Skin(std::string &name) : name(name) {}
    friend class GLTFLoader;
};

