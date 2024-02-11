#include "glm/detail/type_mat4x4.hpp"

struct CameraMatrices
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 viewProj;
    //alignas(16) glm::mat4 proj;
    vk::DescriptorBufferInfo getBufferInfo(Buffer &buffer)
    {
        return {buffer.buffer, 0, sizeof(CameraMatrices)};
    }
};