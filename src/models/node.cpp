#include "node.h"
#include <glm/gtx/transform.hpp>

Node::Node() : parent(nullptr), skin(-1)
{}

void Node::draw(vk::CommandBuffer commandBuffer, Pipeline &pipeline)
{
    for (auto mesh : meshes)
    {
        if (!mesh) continue;
        auto wt = getWorldTransform();
        pipeline.setPushConstant(commandBuffer, &wt, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
        auto &material = *mesh->material;
        pipeline.setPushConstant(commandBuffer, &material, sizeof(Material), sizeof(glm::mat4), vk::ShaderStageFlagBits::eFragment);
        mesh->draw(commandBuffer);
    }
    for (auto &child : children)
    {
        child->draw(commandBuffer, pipeline);
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

void Node::updateJointTransforms(std::vector<Skin> &skins)
{
    if (skin != -1)
    {
        auto inverse = glm::inverse(getWorldTransform());
        auto &s = skins[skin];
        auto jointMatrices = static_cast<glm::mat4*>(s.jointsBuffer->getPointer());
        for (int i = 0; i < s.nodeBindMatrices.size(); i++)
        {
            auto &joint = s.nodeBindMatrices[i];
            jointMatrices[i] = inverse * (joint.node->getWorldTransform() * joint.inverseBindMatrix);
        }
    }
    for (auto &child : children)
    {
        child->updateJointTransforms(skins);
    }
}
