#version 450

layout(std140, set = 0, binding = 0) uniform cameraInfo {
    vec4 position;
    vec4 forward;
    mat4 viewProj;
    mat4 invViewProj;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 cameraPos;
layout(location = 2) out vec4 cameraForward;
layout(location = 3) out mat4 cameraViewProj;

void main()
{
    fragUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(fragUV * 2.0 - 1.0, 0.0, 1.0);
    cameraPos = ubo.position;
    cameraForward = ubo.forward;
    cameraViewProj = ubo.viewProj;
}