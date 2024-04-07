#version 450

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inputPosition;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inputAlbedo;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inputNormal;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput inputMetalRoughnessAO;

layout(location = 0) in vec4 cameraPos;
layout(location = 1) in vec4 cameraForward;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

struct PointLight
{
    vec4 position;
    vec4 color;
    float intensity;
};

// hard coded array of 1 pointlight
PointLight pointLights[2] = PointLight[2](
    PointLight(vec4(0.0, 2.0, 10.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0), 100.0),
    PointLight(vec4(0.0, 2.0, -10.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0), 100.0)
);

const float PI = 3.14159f;

float DistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
    float a2 = pow(roughness, 4.0);
    float nwh2 = pow(dot(normal, halfway), 2.0);
    return a2 / (PI * pow(nwh2 * (a2 - 1.0) + 1, 2.0));
}
//float GeometrySchlickGGX(float NdotV, float roughness)
//{
//    float r = (roughness + 1.0);
//    float k = (r*r) / 8.0;
//
//    float num   = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//
//    return num / denom;
//}
float GeometrySchlickGGX(float cosw, float roughness)
{
    float k = pow((roughness + 1.0), 2.0) / 8.0;
    return cosw / (cosw * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 aces_tonemap(vec3 color)
{
    mat3 m1 = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
    );
    mat3 m2 = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
    );
    vec3 v = m1 * color;
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return pow(clamp(m2 * (a / b), 0.0, 1.0), vec3(1.0 / 2.2));
}

void main()
{
    vec4 position = subpassLoad(inputPosition);
    vec4 albedo = subpassLoad(inputAlbedo);
    vec3 unconvertedNormal = subpassLoad(inputNormal).xyz;
    vec4 metalRoughnessAO = subpassLoad(inputMetalRoughnessAO);

    if (position.a == 0.0)
    {
        outColor = albedo;
        return;
    }

    // convert normal back to [-1, 1] range
    vec3 normal = unconvertedNormal * 2.0 - 1.0;
    float metallic = metalRoughnessAO.r;
    float roughness = metalRoughnessAO.g;
    float ao = metalRoughnessAO.b;

//    float dot = clamp(dot(normalize(-cameraForward.xyz), normalize(normal)), 0, 1);
//    float diff = pow(dot * 0.5 + 0.5, 2.0);
//
//    outColor = vec4(albedo.rgb * diff, 1.0);

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos.xyz - position.xyz);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 2; ++i)
    {
        vec3 lightPos = pointLights[i].position.xyz;
        // calculate per-light radiance
        vec3 L = normalize(lightPos - position.xyz);
        vec3 H = normalize(V + L);
        float distance    = length(lightPos - position.xyz);
        float attenuation = 1.0 / (distance * distance);

        vec3 lightColor = pointLights[i].color.rgb * pointLights[i].intensity;
        vec3 radiance     = lightColor * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo.rgb * ao;
    vec3 color = ambient + Lo;

//    color = aces_tonemap(color);
    color = color / (color + vec3(1.0));
//    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(vec3(metallic), 1.0);
}