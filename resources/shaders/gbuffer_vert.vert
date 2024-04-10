#version 450

layout(std140, set = 0, binding = 0) uniform cameraInfo {
    vec4 position;
    vec4 forward;
    mat4 viewProj;
    mat4 invViewProj;
} ubo;

layout(push_constant) uniform constants {
    mat4 matrix;
} modelTransform;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBiTangent;

// MAKE SURE YOU UPDATE THE PIPELINE BINDINGS TO MATCH THIS
layout(location = 0) in vec3 inPosition; // note that some types use multiple slots
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV;

const vec3[] colors = {
{1.0f, 0.0f, 0.0f},
{0.0f, 1.0f, 0.0f},
{0.0f, 0.0f, 1.0f},
{1.0f, 1.0f, 1.0f}
};

void main()
{
    vec4 worldPos = modelTransform.matrix * vec4(inPosition, 1.0);
    gl_Position = ubo.viewProj * worldPos;
    fragPos = inPosition;
    mat3 matN = transpose(inverse(mat3(modelTransform.matrix)));
    fragNormal = normalize(matN * inNormal);
    fragTangent = normalize(matN * inTangent);
    fragBiTangent = cross(fragNormal, fragTangent);

    fragColor = colors[0];
    fragUV = inUV;
}