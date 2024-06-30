#include "node.h"
#include <glm/gtx/transform.hpp>

Node::Node(Model *model) : parent(nullptr), model(model), skinIndex(-1)
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

void Node::skin(vk::CommandBuffer commandBuffer, Pipeline &pipeline,
                    DescriptorLayoutCache &layoutCache, DescriptorAllocator &allocator)
{
    for (auto mesh : meshes)
    {
        if (!mesh) continue;
        if (!mesh->jointsBuffer || !mesh->weightsBuffer) continue;

        // bind descriptor sets
        auto pos = mesh->getDescriptorInfo(VertexBufferType::POSITION);
        auto joint = mesh->getDescriptorInfo(VertexBufferTypeExt::JOINTS);
        auto weights = mesh->getDescriptorInfo(VertexBufferTypeExt::WEIGHTS);
        auto matrices = model->skins[skinIndex].jointsBuffer->getDescriptorInfo();

        auto outPos = mesh->getDescriptorInfo(VertexBufferTypeExt::SKIN_POSITION);
//        auto outNormal = mesh->getDescriptorInfo(VertexBufferTypeExt::SKIN_NORMAL);

        vk::DescriptorSet set;
        vk::DescriptorSetLayout layout;
        DescriptorBuilder::beginSet(&layoutCache, &allocator)
            .bindBuffer(0, &pos, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
            .bindBuffer(1, &joint, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
            .bindBuffer(2, &weights, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
            .bindBuffer(3, &matrices, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
            .bindBuffer(4, &outPos, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
            .build(set, layout);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline.getPipelineLayout(), 0, {set}, {});

        assert(pos.range % sizeof(glm::vec3) == 0);
        const auto numPositions = pos.range / sizeof(glm::vec3);
        commandBuffer.dispatch(numPositions, 1, 1);
    }
    for (auto &child : children)
    {
        child->skin(commandBuffer, pipeline, layoutCache, allocator);
    }
}

glm::mat4 Node::getLocalTransform() const
{
    return glm::translate(glm::mat4(), transform.translate) *
           glm::mat4_cast(transform.rotation) *
           glm::scale(glm::mat4(), transform.scale);
}

glm::mat4 Node::getWorldTransform() const
{
    if (parent)
    {
        return parent->getWorldTransform() * getLocalTransform();
    }
    return getLocalTransform();
}

void Node::updateJointTransforms()
{
    if (skinIndex != -1)
    {
        auto inverse = glm::inverse(getWorldTransform());
        auto &s = model->skins[skinIndex];
        auto jointMatrices = static_cast<glm::mat4*>(s.jointsBuffer->getPointer());
        for (int i = 0; i < s.nodeBindMatrices.size(); i++)
        {
            auto &joint = s.nodeBindMatrices[i];
            jointMatrices[i] = inverse * (joint.node->getWorldTransform() * joint.inverseBindMatrix);
        }
    }
    for (auto &child : children)
    {
        child->updateJointTransforms();
    }
}
