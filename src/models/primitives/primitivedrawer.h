#include "../../vulkan/pipeline/shader.h"
#include "../../vulkan/pipeline/pipeline.h"
#include "../../smartpointer.h"
#include "primitive.h"
#include "../../vulkan/pipeline/pipelinefactory.h"

class PrimitiveDrawer
{
private:
    static inline uPtr<Shader> primitiveShader;
    static inline uPtr<Pipeline> primitivePipeline;

public:
    static inline uPtr<Primitive> cube, sphere;
    static void create(BufferFactory &bufferFactory, vk::Device device, PipelineBuilder *pipelineFactory, RenderPass *renderPass, int subpass);
    static void destroy();
    static void drawCube(vk::CommandBuffer commandBuffer);
    static void drawCube(vk::CommandBuffer commandBuffer, glm::vec4 color, glm::mat4 transform);
    static void drawSphere(vk::CommandBuffer commandBuffer);
    static void drawSphere(vk::CommandBuffer commandBuffer, glm::vec4 color, glm::mat4 transform);
};
