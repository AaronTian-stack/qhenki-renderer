#include "primitivedrawer.h"

void PrimitiveDrawer::create(BufferFactory &bufferFactory, vk::Device device, PipelineBuilder *pipelineFactory, RenderPass *renderPass, int subpass)
{
    primitiveShader = mkU<Shader>(device, "shaders/primitive.vert.spv", "shaders/primitive.frag.spv");
    pipelineFactory->reset();
    pipelineFactory->addVertexInputBinding({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}); // position
    pipelineFactory->getColorBlending().attachmentCount = 1; // TODO: get rid of this eventually
    primitivePipeline = pipelineFactory->buildPipeline(renderPass, subpass, primitiveShader.get());

    cube = mkU<Primitive>(bufferFactory, "..resources/objs/cube.obj");
    sphere = mkU<Primitive>(bufferFactory, "..resources/objs/sphere.obj");
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
