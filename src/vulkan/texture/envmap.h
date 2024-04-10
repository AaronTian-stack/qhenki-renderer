#pragma once

#include "../buffer/bufferfactory.h"

class EnvironmentMap : public Destroyable
{
private:
    uPtr<Image> cubeMap;
    unsigned int maxMipLevels;

public:
    void create(BufferFactory &bufferFactory, CommandPool &commandPool, QueueManager &queueManager, const char *path);
    void destroy() override;
};
