#include "vulkan/vulkan.h"
#include "../destroyable.h"
#include <vector>

class Shader : public Destroyable
{
private:
    vk::ShaderModule vertShaderModule;
    vk::ShaderModule fragShaderModule;
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

    // vulkan wrapper for shader code
    vk::ShaderModule createShaderModule(const std::string &filePath);

    static vk::PipelineShaderStageCreateInfo vertexStageInfo(const vk::ShaderModule &vertShaderModule);
    static vk::PipelineShaderStageCreateInfo fragmentStageInfo(const vk::ShaderModule &fragShaderModule);

public:
    static std::vector<char> readFile(const std::string &filename);

    Shader(vk::Device device, const char* vertShaderPath, const char* fragShaderPath);

    const std::array<vk::PipelineShaderStageCreateInfo, 2> &getShaderStages() const { return shaderStages; }

    void destroy() override;

    friend class PipelineBuilder;
};
