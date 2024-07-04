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
    mat4 cubeMapRotation;
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
    vec3 f0;
};

// https://seblagarde.wordpress.com/wp-content/uploads/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf

#define PI 3.14159265359

vec3 F_Schlick(vec3 f0, float f90, float u)
{
    return f0 + (f90 - f0) * pow(1.f - u, 5.f);
}
vec3 F_Fresnel(vec3 SpecularColor, float VoH)
{
    vec3 SpecularColorSqrt = sqrt(min(SpecularColor, vec3(0.99, 0.99, 0.99)));
    vec3 n = (1 + SpecularColorSqrt) / (1 - SpecularColorSqrt);
    vec3 g = sqrt(n*n + VoH*VoH - 1);
    return 0.5 * sqrt((g - VoH) / (g + VoH)) * (1 + sqrt(((g + VoH)*VoH - 1) / ((g - VoH)*VoH + 1)));
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float energyBias = mix(0, 0.5, linearRoughness);
    float energyFactor = mix(1.0, 1.0 / 1.51, linearRoughness);
    float fd90 = energyBias + 2.0 * LdotH*LdotH * linearRoughness;
    vec3 f0 = vec3(1.0);
    float lightScatter = F_Schlick(f0, fd90, NdotL).r;
    float viewScatter = F_Schlick(f0, fd90, NdotV).r;

    return lightScatter * viewScatter * energyFactor;
}

float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
    float alphaG2 = alphaG * alphaG;
    alphaG2 = alphaG2 + 0.0000001;
    float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
    float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);

    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

float D_GGX(float NdotH, float m)
{
    // Divide by PI is apply later 
    float m2 = m * m;
    float f = (NdotH * m2 - NdotH) * NdotH + 1;
    return m2 / (f * f);
}

vec3 GetF(float LdotH, vec3 f0)
{
    float f90 = clamp(50.0 * dot(f0, vec3(0.33)), 0.0, 1.0);
    return F_Schlick(f0, f90, LdotH);
}
vec3 GetSpecular(float NdotV, float NdotL, float LdotH, float NdotH, float roughness, vec3 f0, out vec3 F)
{
    F = GetF(LdotH, f0);
    float Vis = V_SmithGGXCorrelated(NdotV, NdotL, roughness);
    float D = D_GGX(NdotH, roughness);
    vec3 Fr = D * F * Vis / PI;
    return Fr;
}
float GetDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float Fd = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, linearRoughness) / PI;
    return Fd;
}

float RectangleSolidAngle(vec3 worldPos, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
    vec3 v0 = p0 - worldPos;
    vec3 v1 = p1 - worldPos;
    vec3 v2 = p2 - worldPos;
    vec3 v3 = p3 - worldPos;

    vec3 n0 = normalize(cross(v0, v1));
    vec3 n1 = normalize(cross(v1, v2));
    vec3 n2 = normalize(cross(v2, v3));
    vec3 n3 = normalize(cross(v3, v0));

    float g0 = acos(dot(-n0, n1));
    float g1 = acos(dot(-n1, n2));
    float g2 = acos(dot(-n2, n3));
    float g3 = acos(dot(-n3, n0));

    return g0 + g1 + g2 + g3 - 2.0 * PI;
}

// https://wickedengine.net/2017/09/area-lights/

// N:	float3 normal
// L:	float3 light vector
// V:	float3 view vector
#define BRDF_MAKE( N, L, V )								\
	const vec3	    H = normalize(L + V);			  		\
	const float		VdotN = abs(dot(N, V)) + 1e-5f;			\
	const float		LdotN = max(0.0, dot(L, N));  			\
	const float		HdotV = max(0.0, dot(H, V));			\
	const float		HdotN = max(0.0, dot(H, N)); 			\
	const float		NdotV = VdotN;					  		\
	const float		NdotL = LdotN;					  		\
	const float		VdotH = HdotV;					  		\
	const float		NdotH = HdotN;					  		\
	const float		LdotH = HdotV;					  		\
	const float		HdotL = LdotH;

// ROUGHNESS:	float surface roughness
// F0:			float3 surface specular color (fresnel f0)
#define BRDF_SPECULAR( ROUGHNESS, F0, F )					\
	GetSpecular(NdotV, NdotL, LdotH, NdotH, ROUGHNESS, F0, F)

// ROUGHNESS:		float surface roughness
#define BRDF_DIFFUSE( ROUGHNESS )							\
	GetDiffuse(NdotV, NdotL, LdotH, ROUGHNESS)

vec3 BRDF_CALCULATE(Material material, float illumuinace, vec3 lightColor, vec3 N, vec3 L, vec3 V)
{
    float roughness = material.roughness;
    vec3 f0 = material.f0;
    BRDF_MAKE(N, L, V);
    // illuminace accounts for NdotL term
    vec3 F;
    vec3 specular = BRDF_SPECULAR(roughness, f0, F) ;

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - material.metallic;

    vec3 diffuse = kD * material.albedo / PI;

    diffuse = max(vec3(0.0), diffuse);
    specular = max(vec3(0.0), specular);

    return (diffuse + specular) * lightColor * illumuinace;
}

vec3 getSpecularDominantDirArea(vec3 N, vec3 R, float roughness)
{
    float lerpFactor = (1.0 - roughness);
    return normalize(mix(N, R, lerpFactor));
}
// horizon handling for sphere and disk
float illuminanceSphereOrDisk(float cosTheta, float sinSigmaSqr)
{
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float illuminance = 0.0f;
    if (cosTheta * cosTheta > sinSigmaSqr)
    {
        illuminance = PI * sinSigmaSqr * clamp(cosTheta, 0.0, 1.0);
    }
    else
    {
        float x = sqrt(1.0f / sinSigmaSqr - 1.0f);
        float y = -x * (cosTheta / sinTheta);
        float sinThetaSqrtY = sinTheta * sqrt(1.0f - y * y);
        illuminance = (cosTheta * acos(y) - x * sinThetaSqrtY) * sinSigmaSqr + atan(sinThetaSqrtY / x);
    }
    return max(illuminance, 0.0f);
}
vec3 closestPointSphere(float radius, vec3 L, vec3 R, vec3 fragPos)
{
    vec3 centerToRay = (dot(L, R) * R) - L;
    vec3 closestPoint = L + centerToRay * clamp(radius / length(centerToRay), 0.0, 1.0);
    return normalize(closestPoint);
}
vec3 calculateSphereLight(SphereLight light, vec3 N, vec3 V, vec3 R, vec3 fragPos, Material material)
{
    vec3 Lunormalized = light.position - fragPos;
    float dist = length(Lunormalized);
    vec3 L = normalize(Lunormalized);

    float sqrDist = dist * dist;

    float cosTheta = clamp(dot(N, L), -0.999, 0.999);   // Clamp to avoid edge case
                                                        // We need to prevent the object penetrating into the surface
                                                        // and we must avoid divide by 0, thus the 0.9999f
    float sqrLightRadius = light.radius * light.radius;
    float sinSigmaSqr = min(sqrLightRadius / sqrDist, 0.9999);
    float illumuinace = illuminanceSphereOrDisk(cosTheta, sinSigmaSqr);

    L = closestPointSphere(light.radius, L, R, fragPos); // MPR calculation

    return BRDF_CALCULATE(material, illumuinace, light.color, N, L, V);
}

vec3 closestPointOnLine(vec3 a, vec3 b, vec3 c)
{
    vec3 ab = b - a;
    float t = dot(c - a , ab) / dot(ab, ab);
    return a + t * ab ;
}
vec3 closestPointOnSegment(vec3 a, vec3 b, vec3 c)
{
    vec3 ab = b - a;
    float t = dot(c - a, ab) / dot(ab, ab);
    return a + clamp (t, 0.0, 1.0) * ab;
}
vec3 calculateTubeLight(TubeLight light, vec3 N, vec3 V, vec3 R, vec3 fragPos, Material material)
{
    float lightWidth = distance(light.position1, light.position2);
    // The sphere is placed at the nearest point on the segment .
    // The rectangular plane is define by the following orthonormal frame :
    vec3 forward = normalize ( closestPointOnLine (light.position1 , light.position2 , fragPos) - fragPos );
    vec3 left = normalize( light.position2 - light.position1 ); // light left
    vec3 up = cross (left, forward);

    vec3 lightPos = mix(light.position1, light.position2, 0.5); // the center point of tube

    vec3 p0 = lightPos - left * (0.5 * lightWidth) + light.radius * up;
    vec3 p1 = lightPos - left * (0.5 * lightWidth) - light.radius * up;
    vec3 p2 = lightPos + left * (0.5 * lightWidth) - light.radius * up;
    vec3 p3 = lightPos + left * (0.5 * lightWidth) + light.radius * up;

    float solidAngle = RectangleSolidAngle ( fragPos , p0 , p1 , p2 , p3 );

    float illuminance = solidAngle * 0.2 * (
        clamp ( dot ( normalize ( p0 - fragPos ) , N ), 0.0 , 1.0) +
        clamp ( dot ( normalize ( p1 - fragPos ) , N ), 0.0 , 1.0) +
        clamp ( dot ( normalize ( p2 - fragPos ) , N ), 0.0 , 1.0) +
        clamp ( dot ( normalize ( p3 - fragPos ) , N ), 0.0 , 1.0) +
        clamp ( dot ( normalize ( lightPos - fragPos ) , N ), 0.0 , 1.0));

    // We then add the contribution of the sphere
    vec3 spherePosition = closestPointOnSegment (light.position1 , light.position2 , fragPos );
    vec3 sphereUnormL = spherePosition - fragPos ;
    vec3 sphereL = normalize ( sphereUnormL );
    float sqrSphereDistance = dot ( sphereUnormL , sphereUnormL );

    float illuminanceSphere = PI * clamp ( dot ( sphereL , N ), 0.0 , 1.0) *
        (  ( light.radius * light.radius ) / sqrSphereDistance );

    illuminance += illuminanceSphere;

    vec3 L = lightPos - fragPos;
    L = closestPointSphere(light.radius, L, R, fragPos); // MPR calculation

    return BRDF_CALCULATE(material, illuminance, light.color, N, L, V);
}

vec3 tracePlane(vec3 planeOrigin, vec3 planeNormal, vec3 fragPos, vec3 R)
{
    float denom = dot(planeNormal, R);
    float t = dot(planeNormal, planeOrigin - fragPos) / denom;
    vec3 planeIntersectionPoint = fragPos + R * t;
    return planeIntersectionPoint;
}
vec3 closestPointOnRectangle(vec3 fragPos, vec3 planeOrigin, vec3 planeright, vec3 planeUp, vec2 halfSize)
{
    vec3 dir = fragPos - planeOrigin;
    vec2 dist2D = vec2(dot(dir, -planeright), dot(dir, planeUp));
    dist2D = clamp(dist2D, -halfSize, halfSize);
    return planeOrigin + planeright * dist2D.x + planeUp * dist2D.y;
}
vec3 calculateRectangle(RectangleLight light, vec3 N, vec3 V, vec3 R, vec3 fragPos, Material material)
{
    vec3 nRight = normalize(light.right);
    vec3 nUp = normalize(light.up);
    vec3 normal = normalize(cross(nRight, nUp));

    vec3 p0 = light.position + light.right + light.up;
    vec3 p1 = light.position + light.right - light.up;
    vec3 p2 = light.position - light.right - light.up;
    vec3 p3 = light.position - light.right + light.up;

    float illuminance = 0;
    float solidAngle = RectangleSolidAngle(fragPos, p0, p1, p2, p3);
    if (dot(fragPos - light.position, normal) > 0) // Same side as plane normal
    {
        illuminance = solidAngle * 0.2 * (
        clamp(dot(normalize(p0 - fragPos), N), 0.0 , 1.0) +
        clamp(dot(normalize(p1 - fragPos), N), 0.0 , 1.0) +
        clamp(dot(normalize(p2 - fragPos), N), 0.0 , 1.0) +
        clamp(dot(normalize(p3 - fragPos), N), 0.0 , 1.0) +
        clamp(dot(normalize(light.position - fragPos), N), 0.0 , 1.0));
    }
    illuminance = max(0, illuminance);

    vec2 halfSize = vec2(length(light.right), length(light.up));
    vec3 nearest3DPoint = closestPointOnRectangle(fragPos, light.position, nRight, nUp, halfSize);

    vec3 L = normalize(nearest3DPoint - fragPos);

    return BRDF_CALCULATE(material, illuminance, light.color, N, L, V);
}

vec3 calculateIBL(vec3 N, vec3 V, vec3 R, vec3 F0, Material material)
{
    vec3 rotatedN = normalize((lightingInfo.cubeMapRotation * vec4(N, 1.0)).xyz);
    vec3 rotatedR = normalize((lightingInfo.cubeMapRotation * vec4(R, 1.0)).xyz);

    R = getSpecularDominantDirArea(N, R, material.roughness);
    float nvDOT = max(dot(N, V), 0.0);

    // assume L = R
    const vec3 H = normalize(V + R);
    vec3 F = GetF(max(dot(R, H), 0.0), F0); // fresnesl schlick

    vec3 kD = 1.0 - F;
    kD *= 1.0 - material.metallic;

    vec3 irradiance = texture(irradianceMap, rotatedN).rgb;
    vec3 diffuse    = irradiance * material.albedo;

    float MAX_REFLECTION_LOD = textureQueryLevels(radianceMap) + 1.0;
    float level = material.roughness * MAX_REFLECTION_LOD;
    vec3 prefilteredColor;
    if (level <= 1)
        prefilteredColor = texture(cubeMap, rotatedR).rgb;
    else
        prefilteredColor = textureLod(radianceMap, rotatedR, material.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF  = texture(brdfLUT, vec2(nvDOT, material.roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular) * material.ao;
    return ambient;
}

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
    R = getSpecularDominantDirArea(N, R, roughness);

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

    Material material = Material(albedo.rgb, metallic, roughness, ao, F0);

    vec3 Lo = vec3(0.0);

    for(int i = 0; i < lightingInfo.sphereLightCount; ++i)
    {
        SphereLight sphereLight = sphereLightBuffer.sphereLights[i];
        sphereLight.color += vec3(0.0000001); // get rid of odd floating point error
        Lo += calculateSphereLight(sphereLight, N, V, R, position.xyz, material);
    }
    for (int i = 0; i < lightingInfo.tubeLightCount; ++i)
    {
        TubeLight tubeLight = tubeLightBuffer.tubeLights[i];
        Lo += calculateTubeLight(tubeLight, N, V, R, position.xyz, material);
    }
    for (int i = 0; i < lightingInfo.rectangleLightCount; ++i)
    {
        RectangleLight rectangleLight = rectangleLightBuffer.rectangleLights[i];
        Lo += calculateRectangle(rectangleLight, N, V, R, position.xyz, material);
    }

    vec3 ambient = calculateIBL(N, V, R, F0, material) * lightingInfo.iblIntensity;
    vec3 color = ambient + Lo + emissive.rgb * lightingInfo.emissionMultiplier;

    outColor = vec4(color.xyz, 1.0);
}