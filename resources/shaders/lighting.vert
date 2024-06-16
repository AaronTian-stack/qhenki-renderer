#version 450
#extension GL_EXT_scalar_block_layout : require

layout(scalar, set = 0, binding = 0) uniform cameraInfo {
    vec3 position;
    vec3 forward;
    mat4 viewProj;
    mat4 invViewProj;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 cameraPos;
layout(location = 2) out vec3 cameraForward;
layout(location = 3) out mat4 cameraViewProj;

void main()
{
    fragUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(fragUV * 2.0 - 1.0, 0.0, 1.0);
    cameraPos = ubo.position;
    cameraForward = ubo.forward;
    cameraViewProj = ubo.viewProj;
}