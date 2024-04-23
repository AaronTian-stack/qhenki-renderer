#pragma once

#include "../../vulkan/buffer/bufferfactory.h"

class Primitive
{
private:
    uPtr<Buffer> positionBuffer;
    uPtr<Buffer> indexBuffer;

public:
    Primitive(BufferFactory &bufferFactory, std::string path);
    void destroy();

    void draw(vk::CommandBuffer commandBuffer);

    friend class PrimitiveDrawer;
};
