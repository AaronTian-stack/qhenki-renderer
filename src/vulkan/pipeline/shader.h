#pragma once

#include <vulkan/vulkan.h>
#include "../destroyable.h"
#include <vector>

class Shader : public Destroyable
{
private:
    vk::ShaderModule vertShaderModule = nullptr;
    vk::ShaderModule fragShaderModule = nullptr;
    vk::ShaderModule compShaderModule = nullptr;
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

    // vulkan wrapper for shader code
    vk::ShaderModule createShaderModule(const std::string &filePath);

public:
    static std::vector<char> readFile(const std::string &filename);

    Shader(vk::Device device, const char* compShaderPath);
    Shader(vk::Device device, const char* vertShaderPath, const char* fragShaderPath);

    const std::array<vk::PipelineShaderStageCreateInfo, 2> &getShaderStages() const { return shaderStages; }

    void destroy() override;

    friend class PipelineBuilder;
};
