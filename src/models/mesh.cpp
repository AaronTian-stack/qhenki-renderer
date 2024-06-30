#include "mesh.h"

Mesh::Mesh() : material(nullptr), jointsBuffer(nullptr), weightsBuffer(nullptr), indexBuffer(nullptr)
{}

void Mesh::destroy()
{
    for (auto &vb : vertexBuffers)
    {
        if (vb) vb->destroy();
    }
    if(indexBuffer) indexBuffer->destroy();
    if (jointsBuffer) jointsBuffer->destroy();
    if (weightsBuffer) weightsBuffer->destroy();
    if (skinnedPositions) skinnedPositions->destroy();
    if (skinnedNormals) skinnedNormals->destroy();
}

void Mesh::draw(vk::CommandBuffer commandBuffer)
{
    // TODO: needs some notion of what shader is bound
    std::vector<Buffer*> buffers;

    buffers.reserve(vertexBuffers.size());
    for (auto &vb : vertexBuffers)
    {
        if (vb) buffers.push_back(vb.get());
    }

    if (skinnedPositions)
        skinnedPositions->bind(commandBuffer, 0);
    else
        vertexBuffers[VertexBufferType::POSITION]->bind(commandBuffer, 0);

    if (skinnedNormals)
        skinnedNormals->bind(commandBuffer, 1);
    else
        vertexBuffers[VertexBufferType::NORMAL]->bind(commandBuffer, 1);

    for (int i = 2; i < vertexBuffers.size(); i++)
    {
        if (vertexBuffers[i])
        {
            vertexBuffers[i]->bind(commandBuffer, i);
        }
        else if (i == VertexBufferType::UV_1)
        {
            vertexBuffers[VertexBufferType::UV_0]->bind(commandBuffer, i);
        }
    }

    indexBuffer->bind(commandBuffer);
    size_t size = indexBuffer->getIndexType() == vk::IndexType::eUint16 ? sizeof(uint16_t) : sizeof(uint32_t);
    auto count = indexBuffer->info.size / size;
    commandBuffer.drawIndexed(count, 1, 0, 0, 0);
}

vk::DescriptorBufferInfo Mesh::getDescriptorInfo(unsigned int bufferType)
{
    if (bufferType >= END) throw std::runtime_error("Invalid buffer type");

    if (bufferType <= UV_1)
        return vertexBuffers[bufferType]->getDescriptorInfo();

    if (bufferType == JOINTS)
        return jointsBuffer->getDescriptorInfo();

    if (bufferType == WEIGHTS)
        return weightsBuffer->getDescriptorInfo();

    if (bufferType == SKIN_POSITION)
        return skinnedPositions->getDescriptorInfo();

    if (bufferType == SKIN_NORMAL)
        return skinnedNormals->getDescriptorInfo();

    return {};
}
