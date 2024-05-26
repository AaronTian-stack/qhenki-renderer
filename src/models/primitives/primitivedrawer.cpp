#include "primitivedrawer.h"
#include <thread>

void PrimitiveDrawer::create(BufferFactory &bufferFactory, vk::Device device, PipelineBuilder &pipelineFactory, RenderPass *renderPass, int subpass)
{
//    primitiveShader = mkU<Shader>(device, "primitive_vert.spv", "primitive_frag.spv");
//    pipelineFactory->reset();
//    pipelineFactory->addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex});
//    pipelineFactory->getColorBlending().attachmentCount = 1; // TODO: get rid of this eventually
//    primitivePipeline = pipelineFactory->buildPipeline(renderPass, subpass, primitiveShader.get());

    auto time1 = std::chrono::high_resolution_clock::now();
    auto cubeF = [&]() {
        cube = mkU<Primitive>(bufferFactory, "../resources/objs/cube.obj");
    };
    auto sphereF = [&]() {
        sphere = mkU<Primitive>(bufferFactory, "../resources/objs/sphere.obj");
    };
    std::thread cubeThread(cubeF);
    std::thread sphereThread(sphereF);
    cubeThread.join();
    sphereThread.join();
    auto time2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();
    std::cout << "Primitive creation time: " << duration << "ms" << std::endl;
}

void PrimitiveDrawer::drawCube(vk::CommandBuffer commandBuffer, glm::vec4 color, glm::mat4 transform)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, primitivePipeline->getGraphicsPipeline());
    primitivePipeline->setPushConstant(commandBuffer, &transform, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
    primitivePipeline->setPushConstant(commandBuffer, &color, sizeof(glm::vec4), sizeof(glm::mat4), vk::ShaderStageFlagBits::eFragment);
    cube->draw(commandBuffer);
}

void PrimitiveDrawer::drawSphere(vk::CommandBuffer commandBuffer, glm::vec4 color, glm::mat4 transform)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, primitivePipeline->getGraphicsPipeline());
    primitivePipeline->setPushConstant(commandBuffer, &transform, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
    primitivePipeline->setPushConstant(commandBuffer, &color, sizeof(glm::vec4), sizeof(glm::mat4), vk::ShaderStageFlagBits::eFragment);
    sphere->draw(commandBuffer);
}

void PrimitiveDrawer::destroy()
{
//    primitivePipeline->destroy();
//    primitiveShader->destroy();
    cube->destroy();
    sphere->destroy();
}

void PrimitiveDrawer::drawCube(vk::CommandBuffer commandBuffer)
{
    cube->draw(commandBuffer);
}

void PrimitiveDrawer::drawSphere(vk::CommandBuffer commandBuffer)
{
    sphere->draw(commandBuffer);
}
