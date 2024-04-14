#include "node.h"
#include "glm/gtx/transform.hpp"

Node::Node() : parent(nullptr), mesh(nullptr), translate(0.f), scale(1.f)
{}

void Node::draw(Node *node, vk::CommandBuffer commandBuffer, Pipeline &pipeline)
{
    auto wt = getWorldTransform();
    if (node->mesh)
    {
        pipeline.setPushConstant(commandBuffer, &wt, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
        auto &material = node->mesh->material;
        pipeline.setPushConstant(commandBuffer, &material, sizeof(Material), sizeof(glm::mat4), vk::ShaderStageFlagBits::eFragment);
        node->mesh->draw(commandBuffer);
    }
    for (auto &child : children)
    {
        child->draw(child.get(), commandBuffer, pipeline);
    }
}

glm::mat4 Node::getLocalTransform()
{
    return glm::translate(glm::mat4(), translate) *
        glm::mat4_cast(rotation) *
        glm::scale(glm::mat4(), scale);
}

glm::mat4 Node::getWorldTransform()
{
    if (parent)
    {
        return parent->getWorldTransform() * getLocalTransform();
    }
    return getLocalTransform();
}

