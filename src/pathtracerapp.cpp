#include "pathtracerapp.h"

PathTracerApp::PathTracerApp()
{

}

PathTracerApp::~PathTracerApp()
{
    // destroy instance last
    vulkanInstance.dispose();
}

void PathTracerApp::dispose()
{
    //pipeline->dispose();
    //renderPass.dispose();

    // destroy swap chain before the device
    swapChainManager.dispose();
    // destroy logical device
    vulkanDevicePicker.dispose();
    // destroy debugger
    vulkanDebugger.dispose();
}

void PathTracerApp::create(Window &window)
{
    VulkanInstance::listExtensions();

    const bool verbose = false;
    // create instance, load extensions
    vulkanInstance.create(verbose);
    // create debugger, validation layers
    vulkanDebugger.create(vulkanInstance, verbose);
    window.createSurface(vulkanInstance);
    // pick physical device (with feature checking)
    vulkanDevicePicker.pickPhysicalDevice(vulkanInstance, window.getSurface());
    // create queues, logical device
    vulkanDevicePicker.createLogicalDevice();
    // retrieve queues
    vulkanQueueManager.initQueues(vulkanDevicePicker.getDevice(), vulkanDevicePicker.selectedDeviceFamily());

    // create swap chain
    swapChainManager.createSwapChain(vulkanDevicePicker, window);

    // create render pass
    renderPass.setDevice(vulkanDevicePicker.getDevice());
    renderPass.create();
    // TODO FIX! : format is MTLPixelFormatRGBA8Uint????? cannot create pipeline
    ////renderPass.setColorAttachmentFormat(swapChainManager.getFormat());

    VulkanShader shader(vulkanDevicePicker.getDevice(), "shader_vert.spv", "shader_frag.spv");

    pipeline = pipelineBuilder.buildPipeline(vulkanDevicePicker.getDevice(), renderPass, shader);
    shader.dispose();
}

void PathTracerApp::render()
{

}

void PathTracerApp::resize()
{

}

VulkanInstance PathTracerApp::getVulkanInstance()
{
    return vulkanInstance;
}
