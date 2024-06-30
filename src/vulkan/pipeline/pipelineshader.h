#pragma once

#include <smartpointer.h>
#include "pipeline.h"
#include "shader.h"

struct PipelineShader
{
    uPtr<Pipeline> pipeline;
    uPtr<Shader> shader;
    void destroy() const {
        pipeline->destroy();
        shader->destroy();
    }
};