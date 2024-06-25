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

vec3 planeVertex[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

layout(scalar, push_constant) uniform ModelTransform {
    mat4 transform;
} modelTransform;

void main()
{
    gl_Position = ubo.viewProj * modelTransform.transform * vec4(planeVertex[gl_VertexIndex], 1.0);
}