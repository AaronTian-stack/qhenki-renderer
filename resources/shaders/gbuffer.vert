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
    mat4 matrix;
} modelTransform;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec2 fragUV_0;
layout(location = 3) out vec2 fragUV_1;
layout(location = 4) out vec3 fragNormal;
layout(location = 5) out vec3 fragTangent;
layout(location = 6) out vec3 fragBiTangent;

// MAKE SURE YOU UPDATE THE PIPELINE BINDINGS TO MATCH THIS
layout(location = 0) in vec3 inPosition; // note that some types use multiple slots
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV_0;
layout(location = 4) in vec2 inUV_1; // second UV set for AO

void main()
{
    vec4 worldPos = modelTransform.matrix * vec4(inPosition, 1.0);
    gl_Position = ubo.viewProj * worldPos;
    fragPos = worldPos.xyz;
    mat3 matN = transpose(inverse(mat3(modelTransform.matrix)));

    fragNormal = matN * inNormal;
    fragTangent = matN * inTangent;
    fragBiTangent = cross(fragNormal, fragTangent);

    fragColor = vec3(1.0, 0.0, 0.0); // for debugging
    fragUV_0 = inUV_0;
    fragUV_1 = inUV_1;
}