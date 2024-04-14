#pragma once

#include "../../vulkan/buffer/bufferfactory.h"
#include "../../vulkan/pipeline/shader.h"
#include "../../vulkan/pipeline/pipeline.h"

class Primitive
{
private:
    static inline uPtr<Buffer> cubePosBuffer;
    static inline uPtr<Buffer> cubeIndexBuffer;
    static inline uPtr<Buffer> spherePosBuffer;
    static inline uPtr<Buffer> sphereIndexBuffer;

    static inline uPtr<Shader> primitiveShader;
    static inline uPtr<Pipeline> primitivePipeline;

    static std::pair<uPtr<Buffer>, uPtr<Buffer>> create(BufferFactory &bufferFactory, std::string path);

public:
    static void create(BufferFactory &bufferFactory);
    static void destroy();
};
