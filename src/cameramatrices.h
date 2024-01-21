#include "glm/detail/type_mat4x4.hpp"
#include "vulkan/bufferstruct.h"

struct CameraMatrices : public BufferStruct
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 model;
    vk::DescriptorBufferInfo getBufferInfo(Buffer &buffer) override
    {
        return {buffer.buffer, 0, sizeof(CameraMatrices)};
    }
};