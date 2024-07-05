#include "shader.h"
#include <string>
#include <fstream>
#include <iostream>

std::vector<char> Shader::readFile(const std::string &filename)
{
    // the working directory may be different if running from command line
//    system("pwd");
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
        std::cerr << "ERROR: SHADER: " << filename << " FILE_NOT_SUCCESFULLY_READ" << std::endl;
        throw std::runtime_error("File not successfully read");
    }
    return {};
}

vk::ShaderModule Shader::createShaderModule(const std::string &filePath)
{
    auto code = readFile(filePath);
    vk::ShaderModuleCreateInfo createInfo(
    vk::ShaderModuleCreateFlags(),
            code.size(),
            reinterpret_cast<const uint32_t *>(code.data())
    );
    return device.createShaderModule(createInfo);
}

Shader::Shader(vk::Device device, const char* compShaderPath) : Destroyable(device)
{
    compShaderModule = createShaderModule(compShaderPath);

    auto compShaderStageInfo = vk::PipelineShaderStageCreateInfo(
            vk::PipelineShaderStageCreateFlags(),
            vk::ShaderStageFlagBits::eCompute,
            compShaderModule,
            "main"
    );

    shaderStages = {compShaderStageInfo};
}

Shader::Shader(vk::Device device, const char* vertShaderPath, const char* fragShaderPath) : Destroyable(device)
{
    vertShaderModule = createShaderModule(vertShaderPath);
    fragShaderModule = createShaderModule(fragShaderPath);

    auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo(
            vk::PipelineShaderStageCreateFlags(),
            vk::ShaderStageFlagBits::eVertex,
            vertShaderModule,
            "main"
     );
    auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo(
            vk::PipelineShaderStageCreateFlags(),
            vk::ShaderStageFlagBits::eFragment,
            fragShaderModule,
            "main"
    );

    shaderStages = {vertShaderStageInfo, fragShaderStageInfo};
}

void Shader::destroy()
{
    if (vertShaderModule)
        device.destroyShaderModule(vertShaderModule);
    if (fragShaderModule)
        device.destroyShaderModule(fragShaderModule);
    if (compShaderModule)
        device.destroyShaderModule(compShaderModule);
}
