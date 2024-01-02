#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<char> Shader::readFile(const std::string &filename)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        file.open(filename, std::ios::ate | std::ios::binary);
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR: SHADER: " << filename << " FILE_NOT_SUCCESFULLY_READ" << std::endl;
        std::runtime_error("File not successfully read");
    }
    return {};
}

VkShaderModule Shader::createShaderModule(const std::string &filePath)
{
    std::vector<char> code = readFile(filePath);
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(deviceForDispose, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

Shader::Shader(VkDevice device, const char* vertShaderPath, const char* fragShaderPath)
{
    deviceForDispose = device;

    vertShaderModule = createShaderModule(vertShaderPath);
    fragShaderModule = createShaderModule(fragShaderPath);

    auto vertShaderStageInfo = vertexStageInfo(vertShaderModule);
    auto fragShaderStageInfo = fragmentStageInfo(fragShaderModule);

    shaderStages = {vertShaderStageInfo, fragShaderStageInfo};
}

VkPipelineShaderStageCreateInfo Shader::vertexStageInfo(const VkShaderModule &vertShaderModule)
{
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // entry point, could combine multiple shaders into one module. however not possible with GLSL
    return vertShaderStageInfo;
}

VkPipelineShaderStageCreateInfo Shader::fragmentStageInfo(VkShaderModule const &fragShaderModule)
{
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    return fragShaderStageInfo;
}

void Shader::dispose()
{
    vkDestroyShaderModule(deviceForDispose, fragShaderModule, nullptr);
    vkDestroyShaderModule(deviceForDispose, vertShaderModule, nullptr);
}
