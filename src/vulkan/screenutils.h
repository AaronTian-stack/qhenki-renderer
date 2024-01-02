#include <vulkan/vulkan.h>

class ScreenUtils
{
public:
    static void setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);
    static void setScissor(VkCommandBuffer commandBuffer, VkExtent2D extent);
};
