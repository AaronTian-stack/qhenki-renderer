#include "vulkancontext.h"
#include <vk-bootstrap/VkBootstrap.h>
#include <iostream>

QueuesIndices VulkanContext::selectQueues(vkb::Device &vkb_device, vk::Device device)
{
    vk::Queue graphicsQueue = nullptr;
    uint32_t graphicsIndex = -1;
    vk::Queue presentQueue = nullptr;
    uint32_t presentIndex = -1;
    vk::Queue transferQueue = nullptr;
    uint32_t transferIndex = -1;

    // by default one queue from each family is already enabled
    auto families = vkb_device.queue_families;
    for (uint32_t index = 0; index < families.size(); index++)
    {
        auto family = families[index];

        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            // graphics implicitly can do everything (except compute)
            if (graphicsQueue == nullptr)
            {
                graphicsQueue = device.getQueue(index, 0);
                presentQueue = device.getQueue(index, 0);
                transferQueue = device.getQueue(index, 0);
                graphicsIndex = index;
                presentIndex = index;
                transferIndex = index;
            }
            else
            {
                // the graphics queue has already been set, but now this queue can be used for transfer
                transferQueue = device.getQueue(index, 0);
                transferIndex = index;
            }
        }
    }
    return {graphicsQueue, presentQueue, transferQueue,
            graphicsIndex, presentIndex, transferIndex};
}

bool VulkanContext::create(Window &window)
{
    vkb::InstanceBuilder builder;
    auto severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    auto inst_ret = builder.set_app_name("Vulkan PBR")
#ifndef NDEBUG
            .request_validation_layers()
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
    )
#endif
    .require_api_version(1, 2).build();

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

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;

    VkPhysicalDeviceFeatures requiredFeatures{};
    requiredFeatures.samplerAnisotropy = VK_TRUE;
    requiredFeatures.shaderInt16 = VK_TRUE;

    vkb::PhysicalDeviceSelector selector{ vkbInstance };

    auto phys_ret = selector.set_surface(surface)
            .set_minimum_version (1, 2)
            .add_required_extensions(deviceExtensions)
            .add_required_extension_features(indexingFeatures)
            .set_required_features(requiredFeatures)
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
            .select();

    if (!phys_ret)
    {
        std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
        return false;
    }

    VkPhysicalDeviceScalarBlockLayoutFeatures scalarFeatures{};
    scalarFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
    scalarFeatures.scalarBlockLayout = VK_TRUE;

//    VkPhysicalDevice16BitStorageFeatures storageFeatures{};
//    storageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
//    storageFeatures.storageBuffer16BitAccess = VK_TRUE;
//    storageFeatures.storageInputOutput16 = VK_TRUE;
//    storageFeatures.storagePushConstant16 = VK_TRUE;
//    storageFeatures.uniformAndStorageBuffer16BitAccess = VK_TRUE;

    // by default, one queue from each family is enabled
    vkb::DeviceBuilder device_builder{ phys_ret.value() };
    auto dev_ret = device_builder
            .add_pNext(&scalarFeatures)
            //.add_pNext(&storageFeatures) // causing issues with device selection
            .build();
    if (!dev_ret)
    {
        std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
        return false;
    }
    vkb::Device vkb_device = dev_ret.value();

    this->device = {
        vkb_device,
        vk::PhysicalDevice(vkb_device.physical_device),
        vk::Device(vkb_device.device)
    };

//    auto queues = selectQueues(vkb_device, device.logicalDevice);
//    auto opt = vkb_device.get_queue(vkb::QueueType::transfer);
//    if (opt.has_value())
//    {
//        queues.transfer = vk::Queue(opt.value());
//    }

    QueuesIndices queues = {};
    queues.graphics = vkb_device.get_queue(vkb::QueueType::graphics).value();
    queues.graphicsIndex = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    auto q_present = vkb_device.get_queue(vkb::QueueType::present);
    if (q_present.has_value())
    {
        queues.present = q_present.value();
        queues.presentIndex = vkb_device.get_queue_index(vkb::QueueType::present).value();
    }
    else
    {
        queues.present = queues.graphics;
        queues.presentIndex = queues.graphicsIndex;
    }
    // try to get dedicated queue for transfer
    auto q_transfer = vkb_device.get_dedicated_queue(vkb::QueueType::transfer);
    if (q_transfer.has_value())
    {
        queues.transfer = q_transfer.value();
        queues.transferIndex = vkb_device.get_dedicated_queue_index(vkb::QueueType::transfer).value();
    }
    else
    {
        queues.transfer = queues.graphics;
        queues.transferIndex = queues.graphicsIndex;
    }

    queueManager.initQueues(queues);

    auto formats = device.physicalDevice.getSurfaceFormatsKHR(surface);
    VkSurfaceFormatKHR format;
    format.format = VK_FORMAT_B8G8R8A8_SRGB;
    format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    vkb::SwapchainBuilder swapchain_builder{ vkb_device };
    auto swap_ret = swapchain_builder
            .set_desired_format(format)
            .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
            .add_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
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
