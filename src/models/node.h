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
    Node *parent;
    Model *model;
    std::string name; // for debugging
    int skinIndex;

public:
    Transform transform;

    std::vector<uPtr<Node>> children;
    std::vector<Mesh*> meshes;

    explicit Node(Model *model);
    glm::mat4 getLocalTransform() const;
    glm::mat4 getWorldTransform() const;
    void updateJointTransforms();
    void draw(vk::CommandBuffer commandBuffer, Pipeline &pipeline);
    void skin(vk::CommandBuffer commandBuffer, Pipeline &pipeline,
                  DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator);
    friend class GLTFLoader;
};