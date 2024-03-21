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

    // sort vertex buffers by type
    std::sort(vertexBuffers.begin(), vertexBuffers.end(), [](const auto &a, const auto &b) {
        return a.second < b.second;
    });

    buffers.reserve(vertexBuffers.size());
    for (auto &vb : vertexBuffers)
    {
        buffers.push_back(vb.first.get());
    }
    bind(commandBuffer, buffers);

    indexBuffer->bind(commandBuffer);
    auto count = indexBuffer->info.size / sizeof(uint16_t);
    commandBuffer.drawIndexed(count, 1, 0, 0, 0);
}
