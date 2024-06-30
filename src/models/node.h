#pragma once

#include <vector>
#include <smartpointer.h>
#include <glm/detail/type_mat.hpp>
#include "mesh.h"
#include <glm/detail/type_mat4x4.hpp>
#include "../vulkan/pipeline/pipeline.h"
#include "transform.h"
#include "skin.h"
#include <glm/gtc/quaternion.hpp>

class Node
{
private:
    Node *parent;
    std::string name; // for debugging
    int skin; // index of skin

public:
    Transform transform;

    std::vector<uPtr<Node>> children;
    std::vector<Mesh*> meshes;

    Node();
    glm::mat4 getLocalTransform() const;
    glm::mat4 getWorldTransform();
    void updateJointTransforms(std::vector<Skin> &skins);
    void draw(vk::CommandBuffer commandBuffer, Pipeline &pipeline);
    friend class GLTFLoader;
};