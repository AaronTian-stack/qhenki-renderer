#pragma once

#include "../buffer/bufferfactory.h"

struct CubeMap
{
    uPtr<Image> image;
    uPtr<Texture> texture;
};

class EnvironmentMap : public Destroyable
{
private:
    unsigned int maxMipLevels;
    CubeMap createCubeMap(BufferFactory &bufferFactory, CommandPool &commandPool, QueueManager &queueManager, const char *path);

public:
    CubeMap cubeMap;
    CubeMap radianceMap;
    CubeMap irradianceMap;

    void create(BufferFactory &bufferFactory, CommandPool &commandPool, QueueManager &queueManager, const char *path);
    void destroy() override;
};
