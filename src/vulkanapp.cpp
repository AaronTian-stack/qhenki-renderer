#include "vulkanapp.h"
#include "vulkan/screenutils.h"
#include "glm/glm.hpp"
#include "inputprocesser.h"
#include "imgui/imgui.h"

VulkanApp::VulkanApp() : camera({0, 2.f, 2.f}) {}

VulkanApp::~VulkanApp()
{
    vulkanContext.device.logicalDevice.waitIdle(); // wait for operations to finish before disposing
    
    for (auto &frame : frames)
        frame.destroy();

    syncer.destroy();
    commandPool.destroy();

    pathPipeline->destroy();
    shader1->destroy(); // shader modules must be destroyed after pipeline

    //triPipeline->destroy();
    shader2->destroy();

    pipelineFactory.destroy();

    renderPass.destroy();

    //buffer->destroy();
    //indexBuffer->destroy();

    for (auto &cameraBuffer : cameraBuffers)
        cameraBuffer->destroy();

    for (auto &allocator : allocators)
        allocator.destroy();

    layoutCache.destroy();
    bufferFactory.destroy();
}

void VulkanApp::create(Window &window)
{
    InputProcesser::setCallbacks(window);

    vulkanContext.create(window);

    layoutCache.create(vulkanContext.device.logicalDevice);
    allocators.emplace_back(vulkanContext.device.logicalDevice);

    auto device = vulkanContext.device.logicalDevice;
    commandPool.create(vulkanContext.device);
    pipelineFactory.create(device);

    bufferFactory.create(vulkanContext);

    // create render pass
    renderPass.setColorAttachmentFormat(vulkanContext.swapChain->getFormat());
    //// CALL CREATE LAST! OR ELSE STUFF WILL BE BROKEN!
    renderPass.create(device);

    shader1 = mkU<Shader>(device, "pathtrace_vert.spv", "pathtrace_frag.spv");
    shader2 = mkU<Shader>(device, "tri_vert.spv", "tri_frag.spv");

    //pipelineFactory.addPushConstant(sizeof(float));
    pipelineFactory.parseFragmentShader("pathtrace_frag.spv");

    pathPipeline = pipelineFactory.buildPipeline(&renderPass, shader1.get());


    //// VERTEX INPUT
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
    };
    const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.f}, {1.0f, 1.0f, 1.0f}}
    };
    const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
    };

    auto stagingBuffer = bufferFactory.createBuffer(
            vertices.size() * sizeof(Vertex),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    stagingBuffer->fill(vertices.data());

    buffer = bufferFactory.createBuffer(vertices.size() * sizeof(Vertex),
                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    stagingBuffer->copyTo(*buffer, vulkanContext.queueManager, commandPool);
    stagingBuffer->destroy();

    buffer = bufferFactory.createBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    buffer->fill(vertices.data());

    indexBuffer = bufferFactory.createBuffer(indices.size() * sizeof(uint16_t),
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    indexBuffer->fill(indices.data());


    //pipelineFactory.reset();

    pipelineFactory.addVertexInputBinding({0, sizeof(Vertex), vk::VertexInputRate::eVertex});
    pipelineFactory.parseVertexShader("tri_vert.spv", layoutCache);
    //// END VERTEX INPUT

    triPipeline = pipelineFactory.buildPipeline(&renderPass, shader2.get());

    vulkanContext.swapChain->createFramebuffers(renderPass.getRenderPass());

    syncer.create(device);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frames.emplace_back(device, commandPool, syncer);
        auto flags = static_cast<VmaAllocationCreateFlagBits>(VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        cameraBuffers.emplace_back(bufferFactory.createBuffer(sizeof(CameraMatrices),
                                                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              flags));
        allocators.emplace_back(vulkanContext.device.logicalDevice);
    }

    //gltfLoad.load();
}

void VulkanApp::recordCommandBuffer(VkFramebuffer framebuffer)
{
    auto &frame = frames[currentFrame];
    auto commandBuffer = frame.commandBuffer;
    auto swapChainExtent = vulkanContext.swapChain->getExtent();

    //updateCameraBuffer();

    //// BEGIN RECORDING COMMAND BUFFER
    frame.begin(); // begins the frames command buffer. for secondary command buffer, need to modify the beginInfo struct

    //// BEGIN RENDER PASS
    renderPass.setFramebuffer(framebuffer);
    renderPass.setRenderAreaExtent(swapChainExtent);
    renderPass.clear(ui->clearColor[0], ui->clearColor[1], ui->clearColor[2], 1.0f);
    renderPass.begin(commandBuffer);

    auto pipeline = ui->currentShaderIndex == 0 ? pathPipeline.get() : triPipeline.get();
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getGraphicsPipeline());
    float time = (float)glfwGetTime();
    pathPipeline->setPushConstant(commandBuffer, &time, sizeof(float));

    ScreenUtils::setViewport(commandBuffer, swapChainExtent.width, swapChainExtent.height);
    ScreenUtils::setScissor(commandBuffer, swapChainExtent);

    if (ui->currentShaderIndex == 1)
    {
        triPipeline->setPushConstant(commandBuffer, &modelTransform, sizeof(glm::mat4));

        buffer->bind(commandBuffer);
        indexBuffer->bind(commandBuffer);

        auto bufferInfo = vk::DescriptorBufferInfo(cameraBuffers[currentFrame]->buffer, 0, sizeof(CameraMatrices));

        auto &allocator = allocators[currentFrame];
        allocator.resetPools();
        vk::DescriptorSet globalSet;
        vk::DescriptorSetLayout layout;
        DescriptorBuilder::begin(&layoutCache, &allocator)
                .bindBuffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
                .build(globalSet, layout);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->getPipelineLayout(), 0, {globalSet}, nullptr);
        commandBuffer.drawIndexed(6, 1, 0, 0, 0);
    }
    else
        commandBuffer.draw(3, 1, 0, 0);

    //// DRAW COMMAND(S)

    ui->end(commandBuffer);

    //// END RENDER PASS
    renderPass.end(); // ends the render pass with the command buffer given in .begin()

    //// END RECORDING COMMAND BUFFER
    frame.end(); // ends the frames command buffer
}

void VulkanApp::render()
{
    handleInput();
    camera.lerp(ImGui::GetIO().DeltaTime);

    auto &frame = frames[currentFrame];
    frame.finish(syncer); // waits for fence to be signalled (finished frame) and resets the frames inFlightFence

    auto commandBuffer = frame.commandBuffer;

    auto &swapChain = vulkanContext.swapChain;
    // gets next available image, returns the corresponding framebuffer and updates internal index to next image
    auto fb = swapChain->nextImage(frame.imageAvailableSemaphore);

    commandBuffer.reset();
    recordCommandBuffer(fb);

    auto &queueManager = vulkanContext.queueManager;
    // lots of information about syncing in frame.getSubmitInfo() struct.
    queueManager.submitGraphics(frame.getSubmitInfo(), frame.inFlightFence); // signal fence when done

    std::vector<vk::Semaphore> signalSemaphores = { frame.renderFinishedSemaphore };
    // calls the present command, waits for given semaphores
    queueManager.present(swapChain, signalSemaphores);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApp::resize()
{

}

ImGuiCreateParameters VulkanApp::getImGuiCreateParameters()
{
    ImGuiCreateParameters params{};
    params.context = &vulkanContext;
    params.renderPass = &renderPass;
    params.framesInFlight = MAX_FRAMES_IN_FLIGHT;
    return params;
}

CommandPool& VulkanApp::getCommandPool()
{
    return commandPool;
}

void VulkanApp::updateCameraBuffer()
{
    auto const options = camera.options;
    auto view = camera.getViewMatrix();
    auto extent = vulkanContext.swapChain->getExtent();
    float aspect = extent.width / (float) extent.height;
    auto proj = glm::perspective(glm::radians(options.fov), aspect, options.nearClip, options.farClip);
    proj[1][1] *= -1;
    cameraMatrices.position = glm::vec4(camera.getPosition(), 1.f);
    cameraMatrices.viewProj = proj * view;
    cameraBuffers[currentFrame]->fill(&cameraMatrices);
}

void VulkanApp::handleInput()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        InputProcesser::setUserPointer(nullptr);
    }
    else {
        InputProcesser::setUserPointer(&camera);
        if (InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_LEFT] ||
             InputProcesser::mouseButtons[GLFW_MOUSE_BUTTON_RIGHT]) {
            InputProcesser::disableCursor();
        }
        else if (InputProcesser::getCursorState() == GLFW_CURSOR_DISABLED) {
            InputProcesser::enableCursor();
        }
    }
}
