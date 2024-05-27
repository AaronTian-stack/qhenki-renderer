#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform PushConstant {
    float intensity;
    float vignetteX;
    float vignetteY;
    float saturation;
    float saturationMul;
} pc;

const vec3 grayscale = vec3(0.3, 0.59, 0.11);

vec3 adjustSaturation(vec3 color, float saturation)
{
    vec3 grey = vec3(dot(color, grayscale));
    return mix(grey, color, saturation);
}

void main()
{
    vec3 color = texture(texSampler, fragUV).rgb;
    float d = distance(fragUV, vec2(0.5));
    float factor = smoothstep(pc.vignetteX, pc.vignetteY, d);
    color = color * factor + color * (1.0 - factor) * (1.0 - pc.intensity);
    color = adjustSaturation(color, pc.saturation) * pc.saturationMul;
    outColor = vec4(color, 1.0);
}