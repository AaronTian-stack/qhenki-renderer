#version 450
#extension GL_EXT_scalar_block_layout : require

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inputAlbedo;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inputMetalRoughnessAO;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput inputEmissive;
layout (input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput depthBuffer;

// radiance is the one with mip levels
layout(set = 2, binding = 0) uniform samplerCube cubeMap;
layout(set = 2, binding = 1) uniform samplerCube irradianceMap;
layout(set = 2, binding = 2) uniform samplerCube radianceMap;
layout(set = 2, binding = 3) uniform sampler2D   brdfLUT;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 cameraPos;
layout(location = 2) in vec3 cameraForward;
layout(location = 3) in mat4 cameraViewProj;

layout(scalar, push_constant) uniform LightingInfo {
    float iblIntensity;
    float emissionMultiplier;
    int pointLightCount;
    int sphereLightCount;
    int tubeLightCount;
    int rectangleLightCount;
} lightingInfo;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

struct Light
{
    vec3 position;
    vec3 color;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
};

struct SphereLight
{
    vec3 position;
    vec3 color;
    float radius;
};

struct TubeLight
{
    vec3 position1;
    vec3 position2;
    vec3 color;
    float radius;
};

struct RectangleLight
{
    vec3 position;
    vec3 up; // not normalized, takes into account size
    vec3 right; // not normalized, takes into account size
    vec3 color;
};

layout(scalar, set = 3, binding = 0) readonly buffer SphereLightBuffer {
    SphereLight sphereLights[];
} sphereLightBuffer;

layout(scalar, set = 3, binding = 1) readonly buffer TubeLightBuffer {
    TubeLight tubeLights[];
} tubeLightBuffer;

layout(scalar, set = 3, binding = 2) readonly buffer RectangleLightBuffer {
    RectangleLight rectangleLights[];
} rectangleLightBuffer;

const float PI = 3.14159;

float DistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
    float a2 = pow(roughness, 4.0);
    float nwh2 = pow(dot(normal, halfway), 2.0);
    return a2 / (PI * pow(nwh2 * (a2 - 1.0) + 1, 2.0));
}
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
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec4 reconstructPosition(float depth)
{
    // vulkan depth is 0 to 1
    vec4 ndc = vec4(fragUV * 2.0 - 1.0, depth, 1.0);
    vec4 pos = inverse(cameraViewProj) * ndc;
    pos /= pos.w;
    return pos;
}

struct Material
{
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

void calculateForLight(inout vec3 Lo, Light light, vec3 N, vec3 V, Material material, vec3 fragPos)
{
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    float distance = distance(light.position, fragPos);
    float attenuation = 1.0 / (distance * distance);

    vec3 radiance = light.color * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, material.roughness);
    float G   = GeometrySmith(N, V, L, material.roughness);
    vec3 F    = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;
}

vec3 closestPointSphere(SphereLight light, vec3 R, vec3 fragPos)
{
    vec3 L = light.position - fragPos;
    vec3 centerToRay = (dot(L, R) * R) - L;
    vec3 closestPoint = L + centerToRay * clamp(light.radius / length(centerToRay), 0.0, 1.0);
    return closestPoint + light.position;
}

vec3 closestPointLineSegment(vec3 a, vec3 b, vec3 p)
{
    vec3 ab = b - a;
    float t = dot(p - a, ab) / dot(ab, ab);
    return a + clamp(t, 0.0, 1.0) * ab;
}

vec3 closestPointTube(TubeLight light, vec3 R, vec3 fragPos)
{
    vec3 closestPointLine = closestPointLineSegment(light.position1, light.position2, fragPos);

    SphereLight s;
    s.radius = light.radius;
    s.position = closestPointLine;
    return closestPointSphere(s, R, fragPos);
}

vec3 tracePlane(vec3 planeOrigin, vec3 planeNormal, vec3 fragPos, vec3 R, out bool hit)
{
    float denom = dot(planeNormal, R);
    float t = dot(planeNormal, planeOrigin - fragPos) / denom;
    vec3 planeIntersectionPoint = fragPos + R * t;

    hit = t > 0.0 && denom < 0.0;
    return planeIntersectionPoint;
}

bool traceTriangle(vec3 a, vec3 b, vec3 c, vec3 p)
{
    vec3 n1 = normalize(cross(b - a, p - b));
    vec3 n2 = normalize(cross(c - b, p - c));
    vec3 n3 = normalize(cross(a - c, p - a));
    float d0 = dot(n1, n2);
    float d1 = dot(n1, n3);
    return d0 > 0.99 && d1 > 0.99;
}

vec3 closestPointRectangle(RectangleLight light, vec3 R, vec3 fragPos, out float t, out bool hit)
{
    // trace the two triangles that make up the rectangle
    vec3 nRight = normalize(light.right);
    vec3 nUp = normalize(light.up);
    vec3 normal = normalize(cross(nRight, nUp));

    vec3 topRightCorner = light.position + light.up + light.right;
    vec3 topLeftCorner = light.position + light.up - light.right;
    vec3 bottomRightCorner = light.position - light.up + light.right;
    vec3 bottomLeftCorner = light.position - light.up - light.right;

    vec3 p = tracePlane(light.position, normal, fragPos, R, hit);
//    bool t0 = traceTriangle(topRightCorner, topLeftCorner, bottomRightCorner, p);
//    bool t1 = traceTriangle(bottomLeftCorner, bottomRightCorner, topLeftCorner, p);
//
//    if (t0 || t1)
//    {
//        hit = true;
//        return p;
//    }
//
//    return vec3(0.);
//
//    // make array of all the points
//    hit = false;
//    vec3 points[4];
//    points[0] = closestPointLineSegment(topRightCorner, topLeftCorner, p);
//    points[1] = closestPointLineSegment(bottomRightCorner, topRightCorner, p);
//    points[2] = closestPointLineSegment(bottomLeftCorner, bottomRightCorner, p);
//    points[3] = closestPointLineSegment(topLeftCorner, bottomLeftCorner, p);
////
//    vec3 np = points[0];
//    float minDist = distance(points[0], p);
//    // find the closest point
//    for (int i = 1; i < 4; i++)
//    {
//        float dist = distance(points[i], p);
//        if (dist < minDist)
//        {
//            np = points[i];
//            minDist = dist;
//        }
//    }
//    return np;
    vec3 intersectionVector = p - light.position;
    vec2 intersectPlanePoint = vec2(dot(intersectionVector, nRight), dot(intersectionVector, nUp));
    float xSize = abs(length(light.right));
    float ySize = abs(length(light.up));
    vec2 nearest2DPoint = vec2(clamp(intersectPlanePoint.x, -xSize, xSize), clamp(intersectPlanePoint.y, -ySize, ySize));
    t = length(nearest2DPoint - intersectPlanePoint);
//    hit = hit && t < 0.0001;
    return light.position + nRight * nearest2DPoint.x + nUp * nearest2DPoint.y;
}

vec3 calculateIBL(vec3 N, vec3 V, vec3 R, vec3 F0, Material material)
{
    float nvDOT = max(dot(N, V), 0.0);
    vec3 F = fresnelSchlickRoughness(nvDOT, F0, material.roughness);

    vec3 kD = 1.0 - F;
    kD *= 1.0 - material.metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * material.albedo;

    float MAX_REFLECTION_LOD = textureQueryLevels(radianceMap) + 1.0;
    float level = material.roughness * MAX_REFLECTION_LOD;
    vec3 prefilteredColor;
    if (level <= 1)
        prefilteredColor = texture(cubeMap, R).rgb;
    else
        prefilteredColor = textureLod(radianceMap, R, material.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF  = texture(brdfLUT, vec2(nvDOT, material.roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular) * material.ao;
    return ambient;
}

const float[16] dither =
{
1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
};

void main()
{
    float depth = subpassLoad(depthBuffer).r;
    vec4 position = reconstructPosition(depth);

    vec4 albedo = subpassLoad(inputAlbedo);
    vec3 normal = subpassLoad(inputNormal).xyz;
    vec4 metalRoughnessAO = subpassLoad(inputMetalRoughnessAO);
    vec4 emissive = subpassLoad(inputEmissive);

    if (depth == 1.0)
    {
        outColor = vec4(0.0);
        return;
    }

    float metallic = metalRoughnessAO.b;
    float roughness = metalRoughnessAO.g;
    float ao = metalRoughnessAO.a;

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos.xyz - position.xyz);
    vec3 R = reflect(-V, N); // reflection vector

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

    Material material = Material(albedo.rgb, metallic, roughness, ao);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lightingInfo.sphereLightCount; ++i)
    {
        SphereLight sphereLight = sphereLightBuffer.sphereLights[i];
        Light light = Light(closestPointSphere(sphereLight, R, position.xyz), sphereLight.color);
        calculateForLight(Lo, light, N, V, material, position.xyz);
    }
    for (int i = 0; i < lightingInfo.tubeLightCount; ++i)
    {
        TubeLight tubeLight = tubeLightBuffer.tubeLights[i];
        float t;
        vec3 c = closestPointTube(tubeLight, R, position.xyz);
        Light light = Light(c, tubeLight.color);
        calculateForLight(Lo, light, N, V, material, position.xyz);
    }
    for (int i = 0; i < lightingInfo.rectangleLightCount; ++i)
    {
        RectangleLight rectangleLight = rectangleLightBuffer.rectangleLights[i];
        bool hit; float t;
        vec3 c = closestPointRectangle(rectangleLight, R, position.xyz, t, hit);
        if (!hit) continue;
        Light light = Light(c, rectangleLight.color);
        // apply linear falloff to light intensity
//        float f = abs(dot(planeNormal, position.xyz - c));
//        float falloff = smoothstep(1.0, 0.0, t) * f;

        vec3 pn = normalize(cross(rectangleLight.right, rectangleLight.up));
        float f2 = abs(dot(pn, normalize(position.xyz - c)));

        float dist = distance(position.xyz, c);
        float falloff = 1.0 - clamp(0.0, 1.0, dist / length(rectangleLight.color));

        light.color *= max(0.0, falloff * f2);
        calculateForLight(Lo, light, N, V, material, position.xyz);
    }
//    for(int i = 0; i < 1; ++i)
//    {
//        Light light = Light(pointLights[i].position.xyz, pointLights[i].color.rgb, pointLights[i].color.a);
//        calculateForLight(Lo, light, N, V, material, position.xyz);
//    }

    vec3 ambient = calculateIBL(N, V, R, F0, material) * lightingInfo.iblIntensity;
    vec3 color = ambient + Lo + emissive.rgb * lightingInfo.emissionMultiplier;

    outColor = vec4(color.xyz, 1.0);
}