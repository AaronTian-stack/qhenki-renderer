#include "vulkancontext.h"
#include <iostream>

bool VulkanContext::create(Window &window)
{
    vkb::InstanceBuilder builder;
    auto severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    auto inst_ret = builder.set_app_name("Vulkan PBR").request_validation_layers()
            .set_debug_messenger_severity(severity)
            .set_debug_callback (
            [] (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void *pUserData)
                    -> VkBool32 {
                std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
                return VK_FALSE;
            }
    ).require_api_version(1, 2).build();

    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
        return false;
    }
    vkbInstance = inst_ret.value();

    auto surface = window.createSurface(vkbInstance.instance);
    this->window = &window;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
    };
#if __APPLE__
    deviceExtensions.push_back("VK_KHR_portability_subset");
#endif

    VkPhysicalDeviceDescriptorIndexingFeatures features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    features.runtimeDescriptorArray = VK_TRUE;
    features.descriptorBindingPartiallyBound = VK_TRUE;
    features.descriptorBindingVariableDescriptorCount = VK_TRUE;

    vkb::PhysicalDeviceSelector selector{ vkbInstance };
    auto phys_ret = selector.set_surface(surface)
            .set_minimum_version (1, 2)
            .add_required_extensions(deviceExtensions)
            .add_required_extension_features(features)
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
            .select();

    if (!phys_ret)
    {
        std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
        return false;
    }

    vkb::DeviceBuilder device_builder{ phys_ret.value() };
    auto dev_ret = device_builder
            .build();
    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
        return false;
    }
    vkb::Device vkb_device = dev_ret.value();

    vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    this->device = {
        vkb_device,
        vk::PhysicalDevice(vkb_device.physical_device),
        vk::Device(vkb_device.device)
    };

    auto graphicsQueueOpt = vkb_device.get_queue(vkb::QueueType::graphics);
    if (!graphicsQueueOpt) {
        std::cerr << "Failed to get graphics queue. Error: " << graphicsQueueOpt.error().message() << "\n";
        return false;
    }
    auto presentQueueOpt = vkb_device.get_queue(vkb::QueueType::present);
    if (!presentQueueOpt) {
        std::cerr << "Failed to get present queue. Error: " << presentQueueOpt.error().message() << "\n";
        return false;
    }
    auto graphicsQueue = vk::Queue(graphicsQueueOpt.value());
    auto presentQueue = vk::Queue(presentQueueOpt.value());

    queueManager.initQueues(graphicsQueue, presentQueue);

    auto formats = device.physicalDevice.getSurfaceFormatsKHR(surface);
    VkSurfaceFormatKHR format;
    format.format = VK_FORMAT_B8G8R8A8_SRGB;
    format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    vkb::SwapchainBuilder swapchain_builder{ vkb_device };
    auto swap_ret = swapchain_builder
            .set_desired_format(format)
            //.set_desired_format()
            .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
            .build();
    if (!swap_ret){
        std::cerr << "Failed to create swap chain. Error: " << swap_ret.error().message() << "\n";
        return false;
    }
    auto swap = swap_ret.value();
    this->swapChain = mkU<SwapChain>(swap);

    return true;
}

VulkanContext::~VulkanContext()
{
    swapChain->destroy();
    vkb::destroy_device(device.vkbDevice);
    vkb::destroy_surface(vkbInstance.instance, window->getSurface());
    vkb::destroy_instance(vkbInstance);
}
