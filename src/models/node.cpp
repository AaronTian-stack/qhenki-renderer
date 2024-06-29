#include "node.h"
#include <glm/gtx/transform.hpp>

Node::Node() : parent(nullptr)
{}

void Node::draw(const uPtr<Node> &node, vk::CommandBuffer commandBuffer, Pipeline &pipeline)
{
    for (auto mesh : node->meshes)
    {
        if (!mesh) continue;
        auto wt = node->getWorldTransform();
        pipeline.setPushConstant(commandBuffer, &wt, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
        auto &material = *mesh->material;
        pipeline.setPushConstant(commandBuffer, &material, sizeof(Material), sizeof(glm::mat4), vk::ShaderStageFlagBits::eFragment);
        mesh->draw(commandBuffer);
    }
    for (auto &child : node->children)
    {
        draw(child, commandBuffer, pipeline);
    }
}

glm::mat4 Node::getLocalTransform() const
{
    return glm::translate(glm::mat4(), transform.translate) *
           glm::mat4_cast(transform.rotation) *
           glm::scale(glm::mat4(), transform.scale);
}

glm::mat4 Node::getWorldTransform()
{
    if (parent)
    {
        return parent->getWorldTransform() * getLocalTransform();
    }
    return getLocalTransform();
}