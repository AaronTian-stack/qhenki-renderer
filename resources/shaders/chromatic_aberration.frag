#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform PushConstant {
    float maxDistortion;
} pc;

vec2 barrelDistortion(vec2 coord, float amt) {
    vec2 cc = coord - 0.5;
    float dist = dot(cc, cc);
    return coord + cc * dist * amt;
}

float sat(float t) {
    return clamp(t, 0.0, 1.0);
}

float linterp(float t) {
    return sat(1.0 - abs(2.0 * t - 1.0));
}

float remap(float t, float a, float b) {
    return sat((t - a) / (b - a));
}

vec4 spectrumOffset(float t) {
    vec4 ret;
    float lo = step(t, 0.5);
    float hi = 1.0 - lo;
    float w = linterp(remap(t, 1.0 / 6.0, 5.0 / 6.0));
    ret = vec4(lo, 1.0, hi, 1.0) * vec4(1.0 - w, w, 1.0 - w, 1.0);

    return pow(ret, vec4(1.0 / pc.maxDistortion));
}

#define NUM_ITER 16
#define RECI_NUM_ITER_F 0.0625

void main()
{
    vec4 sumcol = vec4(0.0);
    vec4 sumw = vec4(0.0);
    for (int i = 0; i < NUM_ITER; ++i) {
        float t = float(i) * RECI_NUM_ITER_F;
        vec4 w = spectrumOffset(t);
        sumw += w;
        sumcol += w * texture(texSampler, barrelDistortion(fragUV, 0.6 * pc.maxDistortion * t));
    }

    outColor = sumcol / sumw;
}