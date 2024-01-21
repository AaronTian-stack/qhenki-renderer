#include <vulkan/vulkan.hpp>
#include "buffer.h"

class BufferStruct
{
public:
    virtual vk::DescriptorBufferInfo getBufferInfo(Buffer &buffer) = 0;
};
