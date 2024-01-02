#include <vector>
#include "vulkan/vulkan.h"
#include "devicepicker.h"
#include "../window.h"
#include "renderpass.h"

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class vec2;

class SwapChainManager : public Disposable
{
private:
    VkDevice deviceForDispose;

    VkSwapchainKHR swapChain;
    // VkImage is a handle to an image object, multidimensional array of data. can be used as attachments, textures, etc.
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews; // TODO: maybe abstraction that pairs image with image view?
    // wraps the image views
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // color channels, color space
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    // buffering
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    // resolution of swap chain images
    static VkExtent2D chooseSwapExtent(Window &window, const VkSurfaceCapabilitiesKHR& capabilities);
    void createImageViews(DevicePicker &vkDevicePicker);

public:
    static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkFormat getFormat() const;
    void createSwapChain(DevicePicker &vkDevicePicker, Window &window);
    void createFramebuffers(RenderPass &renderPass);
    void dispose() override;

    VkExtent2D getExtent() const;
    VkSwapchainKHR getSwapChain();
    VkFramebuffer getFramebuffer(int index);
};
