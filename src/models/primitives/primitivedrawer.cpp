#include "primitivedrawer.h"
#include <thread>

void PrimitiveDrawer::create(BufferFactory &bufferFactory)
{
    auto time1 = std::chrono::high_resolution_clock::now();
    auto cubeF = [&]() {
        cube = mkU<Primitive>(bufferFactory, "../resources/objs/cube.obj");
    };
    auto sphereF = [&]() {
        sphere = mkU<Primitive>(bufferFactory, "../resources/objs/sphere.obj");
    };
    auto cylinderF = [&]() {
        cylinder = mkU<Primitive>(bufferFactory, "../resources/objs/cylinder.obj");
    };
    std::thread cubeThread(cubeF);
    std::thread sphereThread(sphereF);
    std::thread cylinderThread(cylinderF);
    cubeThread.join();
    sphereThread.join();
    cylinderThread.join();
    auto time2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();
    std::cout << "Primitive creation time: " << duration << "ms" << std::endl;
}

void PrimitiveDrawer::setPushConstants(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 *color,
                                       glm::mat4 *transform)
{
    primitivePipeline.setPushConstant(commandBuffer, transform, sizeof(glm::mat4), 0, vk::ShaderStageFlagBits::eVertex);
    primitivePipeline.setPushConstant(commandBuffer, color, sizeof(glm::vec4), sizeof(glm::mat4), vk::ShaderStageFlagBits::eFragment);
}

void PrimitiveDrawer::drawShape(Primitive *p, vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, primitivePipeline.getGraphicsPipeline());
    setPushConstants(commandBuffer, primitivePipeline, &color, &transform);
    p->draw(commandBuffer);
}

void PrimitiveDrawer::drawCube(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform)
{
    drawShape(cube.get(), commandBuffer, primitivePipeline, color, transform);
}

void PrimitiveDrawer::drawSphere(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform)
{
    drawShape(sphere.get(), commandBuffer, primitivePipeline, color, transform);
}

void PrimitiveDrawer::drawCylinder(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform)
{
    drawShape(cylinder.get(), commandBuffer, primitivePipeline, color, transform);
}

void PrimitiveDrawer::destroy()
{
    cube->destroy();
    sphere->destroy();
    cylinder->destroy();
}

void PrimitiveDrawer::drawCube(vk::CommandBuffer commandBuffer)
{
    cube->draw(commandBuffer);
}

void PrimitiveDrawer::drawSphere(vk::CommandBuffer commandBuffer)
{
    sphere->draw(commandBuffer);
}

void PrimitiveDrawer::drawCylinder(vk::CommandBuffer commandBuffer)
{
    cylinder->draw(commandBuffer);
}