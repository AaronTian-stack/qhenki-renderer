#pragma once

#include <vector>
#include <smartpointer.h>
#include "mesh.h"
#include "../vulkan/pipeline/pipeline.h"
#include "transform.h"
#include "skin.h"
#include "../vulkan/descriptors/descriptorbuilder.h"
#include "model.h"

class Node
{
private:
    Transform transform;
    glm::mat4 worldTransform;
    bool dirty;

    Node *parent;
    Model *model;
    std::string name; // for debugging
    int skinIndex;

    std::vector<uPtr<Node>> children;
    std::vector<Mesh*> meshes;

    void invalidate();

public:
    explicit Node(Model *model);
    void setTranslation(glm::vec3 translation);
    void setRotation(glm::quat rotation);
    void setScale(glm::vec3 scale);
    glm::mat4 getLocalTransform();
    glm::mat4 getWorldTransform();
    void updateJointTransforms(int frame);
    void draw(vk::CommandBuffer commandBuffer, Pipeline &pipeline);
    void skin(vk::CommandBuffer commandBuffer, Pipeline &pipeline,
                  DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator, int frame);
    friend class GLTFLoader;
};