#include "node.h"
#include "glm/gtx/transform.hpp"

Node::Node() : parent(nullptr), mesh(nullptr), translate(0.f), scale(1.f)
{}

void Node::draw(vk::CommandBuffer commandBuffer, Pipeline &pipeline, Node &node)
{
    auto wt = node.getWorldTransform();
    if (node.mesh)
    {
        pipeline.setPushConstant(commandBuffer, &wt, sizeof(glm::mat4), 0);
        node.mesh->draw(commandBuffer);
    }
    for (auto &child : node.children)
    {
        draw(commandBuffer, pipeline, *child);
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

