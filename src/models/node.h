#pragma once

#include <vector>
#include "../smartpointer.h"
#include "glm/detail/type_mat.hpp"
#include "mesh.h"
#include "glm/detail/type_mat4x4.hpp"
#include "../vulkan/pipeline/pipeline.h"
#include "glm/gtc/quaternion.hpp"

class Node
{
private:
    Node *parent;
    std::string name; // for debugging

public:
    glm::vec3 translate;
    glm::quat rotation;
    glm::vec3 scale;

    std::vector<uPtr<Node>> children;
    Mesh *mesh;

    Node();
    glm::mat4 getLocalTransform();
    glm::mat4 getWorldTransform();
    void draw(vk::CommandBuffer commandBuffer, Pipeline &pipeline, Node &node);
    friend class GLTFLoader;
};
