#version 450

layout(std140, set = 0, binding = 0) uniform cameraInfo {
    vec4 position;
    vec4 forward;
    mat4 viewProj;
    mat4 invViewProj;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform constants {
    mat4 matrix;
} modelTransform;

layout(location = 0) out vec3 localPos;

// MAKE SURE YOU UPDATE THE PIPELINE BINDINGS TO MATCH THIS
layout(location = 0) in vec3 inPosition;

void main()
{
    localPos = inPosition;
    mat4 rotView = mat4(mat3(ubo.view)); // remove translation from the view matrix
    vec4 clipPos = ubo.proj * rotView * vec4(localPos, 1.0);
    gl_Position = clipPos.xyww; // always depth 1.0
}