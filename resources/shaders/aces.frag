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

vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return mix(vec3(0.0), vec3(1.0), (x*(a*x+b))/(x*(c*x+d)+e));
}

void main()
{
    vec4 color = texture(texSampler, fragUV);
    vec3 aces = ACESFilm(color.rgb);
    dither(aces);
    outColor = vec4(aces, color.a);
}