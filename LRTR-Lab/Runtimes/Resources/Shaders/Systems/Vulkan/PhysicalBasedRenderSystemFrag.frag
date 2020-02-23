#version 450
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.14159265359

struct Material
{
	vec4 BaseColor;
	vec4 Roughness;
	vec4 Metallic;
    vec4 Emissive;
};

struct Light
{
	vec4 Position;
	vec4 Intensity;

    float FarPlane;
	uint Index;
	uint Type;
	uint Unused;
};

layout (push_constant) uniform Config
{
    uint HasEnvironmentLight;
    uint HasBaseColor;
    uint HasRoughness;
    uint HasOcclusion;
    uint HasNormalMap;
    uint HasMetallic;
    uint HasEmissive;
    float EyePositionX;
    float EyePositionY;
    float EyePositionZ;
    uint MipLevels;
    uint Lights;
    uint Index;
} config;

vec3 lerp(vec3 s, vec3 e, float t)
{
    return s * (1.0 - t) + e * t;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 normal, vec3 halfVector, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float normalDotHalf = max(dot(normal, halfVector), 0.0);
	float normalDotHalf2 = normalDotHalf * normalDotHalf;

	float numerator = a2;
	float denominator = (normalDotHalf2 * (a2 - 1.0) + 1.0);

	denominator = PI * denominator * denominator;

	return numerator / denominator;
}

float GeometrySchlickGGX(float normalDot, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float numerator = normalDot;
	float denominator = normalDot * (1.0 - k) + k;

	return numerator / denominator;
}

float GeometrySmith(vec3 normal, vec3 toEye, vec3 lightVector, float roughness)
{
	float normalDotEye = max(dot(normal, toEye), 0.0);
	float normalDotLight = max(dot(normal, lightVector), 0.0);
	float ggx2 = GeometrySchlickGGX(normalDotEye, roughness);
	float ggx1 = GeometrySchlickGGX(normalDotLight, roughness);

	return ggx1 * ggx2;
}

vec3 CookTorranceBRDF(Material material, vec3 radiance, vec3 lightVector, vec3 normal, vec3 toEye, vec3 F0)
{
	//see https://github.com/JoeyDeVries/LearnOpenGL to learn more about PBR and BRDF
	vec3 halfVector = normalize(toEye + lightVector);

	float NDF = DistributionGGX(normal, halfVector, material.Roughness.a);
	float G = GeometrySmith(normal, toEye, lightVector, material.Roughness.a);
	vec3 F = FresnelSchlick(max(dot(halfVector, toEye), 0.0), F0);

	vec3 kS = F;
	vec3 kD = 1.0f - kS;

	kD = kD * (1.0 - material.Metallic.a);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, toEye), 0.0) * max(dot(normal, lightVector), 0.0);

	return (kD * material.BaseColor.rgb / PI + numerator / max(denominator, 0.001)) * radiance;
}

vec3 ComputePointLight(Light light, Material material, vec3 position, vec3 normal, vec3 toEye, vec3 F0)
{
	vec3 lightVector = normalize(light.Position.xyz - position);
	
	float ndotl = max(dot(lightVector, normal), 0.0f);
	float distance = length(light.Position.xyz - position);
	float attenuation = 1.0f / (distance * distance);
	
	return CookTorranceBRDF(material, light.Intensity.xyz * attenuation * ndotl, lightVector, normal, toEye, F0);
}

float GammaCorrect(float value)
{
    if (value <= 0.0031308f) return 12.92f * value;
	
    return 1.055f * pow(value, 1.f / 2.4f) - 0.055f;
}

vec3 GammaCorrect(vec3 value)
{
    return vec3(
		GammaCorrect(value.x),
		GammaCorrect(value.y),
		GammaCorrect(value.z));
}

float InverseGammaCorrect(float value)
{
    if (value <= 0.04045f) return value * 1.f / 12.92f;
    return pow((value + 0.055f) * 1.f / 1.055f, 2.4f);
}

vec3 InverseGammaCorrect(vec3 value)
{
    return vec3(
		InverseGammaCorrect(value.x),
		InverseGammaCorrect(value.y),
		InverseGammaCorrect(value.z));
}

layout (set = 0, binding = 0) buffer Materials
{
    Material Value[];
} materials;

layout (set = 0, binding = 1) buffer Lights
{
    Light Value[];
} lights;

layout (set = 0, binding = 4) uniform texture2D metallicTexture;
layout (set = 0, binding = 5) uniform texture2D baseColorTexture;
layout (set = 0, binding = 6) uniform texture2D roughnessTexture;
layout (set = 0, binding = 7) uniform texture2D occlusionTexture;
layout (set = 0, binding = 8) uniform texture2D normalMapTexture;
layout (set = 0, binding = 9) uniform texture2D emissiveTexture;
layout (set = 0, binding = 10) uniform textureCube irradianceMap;
layout (set = 0, binding = 11) uniform textureCube preFilteringMap;
layout (set = 0, binding = 12) uniform texture2D preComputingBRDF;
layout (set = 0, binding = 13) uniform textureCubeArray pointShadowMaps;

layout (set = 1, binding = 0) uniform sampler materialSampler;

vec3 getNormal(vec3 normal, vec2 texcoord, vec3 tangent)
{
    if (config.HasNormalMap == 0) return normal;

    vec3 tangentNormal = texture(sampler2D(normalMapTexture, materialSampler), texcoord).xyz * 2.0 - 1.0;
    
    vec3 N = normalize(normal);
    vec3 T = normalize(tangent - dot(tangent, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    return TBN * tangentNormal;
}

float ShadowCalculation(vec3 position, uint index)
{
	vec3 lightPosition = lights.Value[index].Position.xyz;
	vec3 fragToLight = position - lightPosition;

	float depth = texture(samplerCubeArray(pointShadowMaps, materialSampler), vec4(position, index)).r;
	float closest = depth * lights.Value[index].FarPlane;

	float current = length(lightPosition);

	float bias = 0.05;

	return current - bias > closest ? 1 : 0;
}

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTexcoord;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec4 outColor;

void main()
{
    vec3 toEye = normalize(vec3(config.EyePositionX, config.EyePositionY, config.EyePositionZ) - inPosition);
    vec3 F0 = vec3(0.04);
	vec3 color = vec3(0.0, 0.0, 0.0);
    float occlusion = 1.0;

    Material material = materials.Value[config.Index];
    material.Roughness = max(material.Roughness, 0.05);
    
    if (config.HasMetallic != 0) material.Metallic.a *= texture(sampler2D(metallicTexture, materialSampler), inTexcoord.xy).b;
    if (config.HasBaseColor != 0) material.BaseColor.rgb *= InverseGammaCorrect(texture(sampler2D(baseColorTexture, materialSampler), inTexcoord.xy).rgb);
    if (config.HasRoughness != 0) material.Roughness.a *= texture(sampler2D(roughnessTexture, materialSampler), inTexcoord.xy).g;
    if (config.HasOcclusion != 0) occlusion = texture(sampler2D(occlusionTexture, materialSampler), inTexcoord.xy).r;
    if (config.HasEmissive != 0) material.Emissive.rgb *= InverseGammaCorrect(texture(sampler2D(emissiveTexture, materialSampler), inTexcoord.xy).rgb);

    F0 = lerp(F0, material.BaseColor.xyz, material.Metallic.a);
	
    vec3 normal = normalize(getNormal(inNormal, inTexcoord.xy, inTangent));
    
    for (uint index = 0; index < config.Lights; index++)
	{
		color = color + ComputePointLight(lights.Value[index], material, inPosition, normal, toEye, F0) * 
			(1.0 - ShadowCalculation(inPosition, index));
	}

    if (config.HasEnvironmentLight != 0)
    {
        vec3 N = normal;
        vec3 V = toEye;
        vec3 R = reflect(-V, N);
		
        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, material.Roughness.a);
		
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - material.Metallic.a);
		
        vec3 irradiance = texture(samplerCube(irradianceMap, materialSampler), N).rgb;
        vec3 diffuse = irradiance * material.BaseColor.rgb;
		
        float MaxLod = config.MipLevels - 1.0;
        float NdotV = max(dot(N, V), 0.0);

        vec3 preFilteringColor = textureLod(samplerCube(preFilteringMap, materialSampler), R, material.Roughness.a * MaxLod).rgb;
        vec2 brdf = texture(sampler2D(preComputingBRDF, materialSampler), vec2(NdotV, material.Roughness.a)).rg; 
        vec3 specular = preFilteringColor * (F * brdf.x + brdf.y);
		
        color = color + (kD * diffuse + specular) * occlusion;
    }else {
        color = color + material.BaseColor.rgb * vec3(0.03) * occlusion;
    }
	
    color = color + material.Emissive.rgb;
	
    color = GammaCorrect(color);

    outColor = vec4(color, material.BaseColor.a);
}