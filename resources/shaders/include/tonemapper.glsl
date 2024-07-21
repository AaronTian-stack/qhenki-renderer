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