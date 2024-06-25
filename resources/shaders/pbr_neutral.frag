#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, set = 1, binding = 0) uniform DitherMatrix {
    float dither[64];
} ditherMatrix;

void dither(inout vec3 color)
{
    // Apply dithering to get rid of banding
    float dither = ditherMatrix.dither[int(mod(gl_FragCoord.y, 8.0)) * 8 + int(mod(gl_FragCoord.x, 8.0))];
    color.rgb = color.rgb + dither / 255.0;
}

vec3 PBRNeutralToneMapping(vec3 color) {
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return color;

    const float d = 1. - startCompression;
    float newPeak = 1. - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
    return mix(color, newPeak * vec3(1, 1, 1), g);
}

void main()
{
    vec4 color = texture(texSampler, fragUV);
    color.rgb = PBRNeutralToneMapping(color.rgb);
    dither(color.rgb);
    outColor = color;
}