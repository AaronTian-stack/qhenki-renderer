#pragma once

#include "../buffer/bufferfactory.h"

struct ImageTexture
{
    uPtr<Image> image;
    uPtr<Texture> texture;
};

class EnvironmentMap : public Destroyable
{
private:
    std::vector<uPtr<Buffer>> stagingBuffers;
    unsigned int maxMipLevels;
    ImageTexture createCubeMap(vk::CommandBuffer *commandBuffer, BufferFactory &bufferFactory, CommandPool &commandPool,
                               const char *path);;

public:
    ImageTexture cubeMap;
    ImageTexture radianceMap;
    ImageTexture irradianceMap;
    ImageTexture brdfLUT;

    void create(BufferFactory &bufferFactory, CommandPool &commandPool, QueueManager &queueManager, const char *path);
    void destroy() override;
};
