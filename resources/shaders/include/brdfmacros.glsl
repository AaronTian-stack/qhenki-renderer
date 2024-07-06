// https://wickedengine.net/2017/09/area-lights/

#define PI 3.14159265359

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