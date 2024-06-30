#pragma once

#include "../../vulkan/pipeline/shader.h"
#include "../../vulkan/pipeline/pipeline.h"
#include <smartpointer.h>
#include "primitive.h"
#include "../../vulkan/pipeline/pipelinefactory.h"

class PrimitiveDrawer
{
private:
    static inline uPtr<Primitive> cube, sphere, cylinder;
    static void setPushConstants(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 *color, glm::mat4 *transform);
    static void drawShape(Primitive *p, vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform);

public:
    static void create(BufferFactory &bufferFactory);
    static void destroy();
    // assumes you have a pipeline bound. only binds position
    static void drawCube(vk::CommandBuffer commandBuffer);
    static void drawCube(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform);
    // assumes you have a pipeline bound. only binds position
    static void drawSphere(vk::CommandBuffer commandBuffer);
    static void drawSphere(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform);

    static void drawCylinder(vk::CommandBuffer commandBuffer);
    static void drawCylinder(vk::CommandBuffer commandBuffer, Pipeline &primitivePipeline, glm::vec4 color, glm::mat4 transform);
};
