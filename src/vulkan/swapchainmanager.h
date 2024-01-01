#include <vector>
#include "vulkan/vulkan.h"
#include "vkdevicepicker.h"
#include "../window.h"

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SwapChainManager : public Disposable
{
private:
    VkDevice deviceForDispose;

    VkSwapchainKHR swapChain;
    // VkImage is a handle to an image object, multidimensional array of data. can be used as attachments, textures, etc.
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

    // color channels, color space
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    // buffering
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    // resolution of swap chain images
    static VkExtent2D chooseSwapExtent(Window &window, const VkSurfaceCapabilitiesKHR& capabilities);
    void createImageViews(VkDevicePicker &vkDevicePicker);

public:
    static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkFormat getFormat() const;
    void createSwapChain(VkDevicePicker &vkDevicePicker, Window &window);
    void dispose() override;
};
