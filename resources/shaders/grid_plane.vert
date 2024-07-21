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

layout(scalar, push_constant) uniform ModelTransform {
    mat4 transform;
} modelTransform;

layout(location = 0) out vec3 camPos;
layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out mat4 viewProj;

vec3 UnprojectPoint(vec2 xy, float z)
{
    vec4 unprojectedPoint = ubo.invViewProj * vec4(xy, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
    camPos = ubo.position;
    fragUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 p = vec4(fragUV * 2.0 - 1.0, 0.0, 1.0);
    nearPoint = UnprojectPoint(p.xy, 0.0).xyz;
    farPoint = UnprojectPoint(p.xy, 1.0).xyz;
    viewProj = ubo.viewProj;
    gl_Position = p;
}