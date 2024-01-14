#include <vulkan/vulkan.hpp>

class ScreenUtils
{
public:
    static void setViewport(vk::CommandBuffer commandBuffer, uint32_t width, uint32_t height);
    static void setScissor(vk::CommandBuffer commandBuffer, vk::Extent2D extent);
};
