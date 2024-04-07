#version 450

layout(std140, set = 0, binding = 0) uniform cameraInfo {
    vec4 position;
    vec4 forward;
    mat4 viewProj;
} ubo;

layout(location = 0) out vec4 forward;

void main()
{
    vec2 fragUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(fragUV * 2.0 - 1.0, 0.0, 1.0);
    forward = ubo.forward;
}