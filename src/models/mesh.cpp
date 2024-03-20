#include "mesh.h"

Mesh::Mesh()
{

}

void Mesh::destroy()
{
    for (auto &vb : vertexBuffers)
    {
        vb.first->destroy();
    }
    indexBuffer->destroy();
}

void Mesh::draw(vk::CommandBuffer commandBuffer)
{
    std::vector<Buffer*> buffers;
    // TODO: need to bind in right order
    buffers.reserve(vertexBuffers.size());
    for (auto &vb : vertexBuffers)
    {
        buffers.push_back(vb.first.get());
    }
    bind(commandBuffer, buffers);
    indexBuffer->bind(commandBuffer);
    commandBuffer.drawIndexed(indexBuffer->info.size / sizeof(uint16_t), 1, 0, 0, 0);
}
