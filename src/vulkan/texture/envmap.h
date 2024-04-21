#pragma once

#include "../buffer/bufferfactory.h"

class EnvironmentMap : public Destroyable
{
private:

    unsigned int maxMipLevels;

public:
    uPtr<Image> cubeImage;
    uPtr<Texture> cubeMap;

    void create(BufferFactory &bufferFactory, CommandPool &commandPool, QueueManager &queueManager, const char *path);
    void destroy() override;
};
