#pragma once

#include <vector>
#include "../vulkan/buffer/buffer.h"
#include "material.h"

class Model;

enum VertexBufferType
{
    POSITION,
    NORMAL,
    TANGENT,
    UV_0,
    UV_1,
};

enum VertexBufferTypeExt
{
    JOINTS = 5,
    WEIGHTS,
    SKIN_POSITION,
    SKIN_NORMAL,
    END
};

class Mesh
{
private:;
    std::array<uPtr<Buffer>, 5> vertexBuffers;

    uPtr<Buffer> jointsBuffer;
    uPtr<Buffer> weightsBuffer;

    uPtr<Buffer> skinnedPositions;
    uPtr<Buffer> skinnedNormals;

    uPtr<Buffer> indexBuffer;

public:
    Material *material;
    Mesh();
    void draw(vk::CommandBuffer commandBuffer);
    vk::DescriptorBufferInfo getDescriptorInfo(unsigned int bufferType);
    void destroy();

    friend class Node;
    friend class GLTFLoader;
};
