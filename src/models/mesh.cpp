#include "mesh.h"

Mesh::Mesh() : material(nullptr), jointsBuffer(nullptr), weightsBuffer(nullptr), indexBuffer(nullptr)
{}

void Mesh::destroy()
{
    for (auto &vb : vertexBuffers)
    {
        if (vb) vb->destroy();
    }
    indexBuffer->destroy();
    if (jointsBuffer) jointsBuffer->destroy();
    if (weightsBuffer) weightsBuffer->destroy();
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

    for (int i = 0; i < vertexBuffers.size(); i++)
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
