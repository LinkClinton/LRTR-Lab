#pragma pack_matrix(row_major)

#define PI 3.14159265359

struct Material
{
	float4 BaseColor;
	float4 Roughness;
	float4 Metallic;
    float4 Emissive;
};

struct Light
{
	float3 Position;
	float3 Intensity;
};

struct Config
{
    uint HasIrradiance;
	uint HasBaseColor;
	uint HasRoughness;
	uint HasOcclusion;
	uint HasNormalMap;
	uint HasMetallic;
    uint HasEmissive;
    float EyePositionX;
	float EyePositionY;
    float EyePositionZ;
	uint Lights;
	uint Index;
};

float3 mix(float3 x, float3 y, float3 a)
{
	return x * (1.0 - a) + y * a;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 normal, float3 halfVector, float roughness)
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

float GeometrySmith(float3 normal, float3 toEye, float3 lightVector, float roughness)
{
	float normalDotEye = max(dot(normal, toEye), 0.0);
	float normalDotLight = max(dot(normal, lightVector), 0.0);
	float ggx2 = GeometrySchlickGGX(normalDotEye, roughness);
	float ggx1 = GeometrySchlickGGX(normalDotLight, roughness);

	return ggx1 * ggx2;
}

float3 CookTorranceBRDF(Material material, float3 radiance, float3 lightVector, float3 normal, float3 toEye, float3 F0)
{
	//see https://github.com/JoeyDeVries/LearnOpenGL to learn more about PBR and BRDF
	float3 halfVector = normalize(toEye + lightVector);

	float NDF = DistributionGGX(normal, halfVector, material.Roughness.a);
	float G = GeometrySmith(normal, toEye, lightVector, material.Roughness.a);
	float3 F = FresnelSchlick(max(dot(halfVector, toEye), 0.0), F0);

	float3 kS = F;
	float3 kD = 1.0f - kS;

	kD = kD * (1.0 - material.Metallic.a);

	float3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, toEye), 0.0) * max(dot(normal, lightVector), 0.0);

	return (kD * material.BaseColor.rgb / PI + numerator / max(denominator, 0.001)) * radiance;
}

float3 ComputePointLight(Light light, Material material, float3 position, float3 normal, float3 toEye, float3 F0)
{
	float3 lightVector = normalize(light.Position - position);
	
	float ndotl = max(dot(lightVector, normal), 0.0f);
	float distance = length(light.Position - position);
	float attenuation = 1.0f / (distance * distance);
	
	return CookTorranceBRDF(material, light.Intensity * attenuation * ndotl, lightVector, normal, toEye, F0);
}

float GammaCorrect(float value)
{
    if (value <= 0.0031308f) return 12.92f * value;
	
    return 1.055f * pow(value, 1.f / 2.4f) - 0.055f;
}

float3 GammaCorrect(float3 value)
{
    return float3(
		GammaCorrect(value.x),
		GammaCorrect(value.y),
		GammaCorrect(value.z));
}

float InverseGammaCorrect(float value)
{
    if (value <= 0.04045f) return value * 1.f / 12.92f;
    return pow((value + 0.055f) * 1.f / 1.055f, 2.4f);
}

float3 InverseGammaCorrect(float3 value)
{
    return float3(
		InverseGammaCorrect(value.x),
		InverseGammaCorrect(value.y),
		InverseGammaCorrect(value.z));
}

StructuredBuffer<Material> materials : register(t0);
StructuredBuffer<Light> lights : register(t1);

Texture2D metallicTexture : register(t4);
Texture2D baseColorTexture : register(t5);
Texture2D roughnessTexture : register(t6);
Texture2D occlusionTexture : register(t7);
Texture2D normalMapTexture : register(t8);
Texture2D emissiveTexture : register(t9);
TextureCube irradianceMap : register(t10);

ConstantBuffer<Config> config : register(b11);

SamplerState materialSampler : register(s12);

float3 getNormal(float3 normal, float2 texcoord, float3 tangent)
{
    if (config.HasNormalMap == 0) return normal;

    float3 tangentNormal = normalMapTexture.Sample(materialSampler, texcoord).xyz * 2.0 - 1.0;
    
    float3 N = normalize(normal);
    float3 T = normalize(tangent - dot(tangent, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);

    return mul(tangentNormal, TBN);
}

float4 main(
	float4 svPosition : SV_POSITION,
	float3 position : POSITION,
	float3 texCoord : TEXCOORD,
	float3 tangent : TANGENT,
	float3 normal : NORMAL) : SV_TARGET
{
    float3 toEye = normalize(float3(config.EyePositionX, config.EyePositionY, config.EyePositionZ) - position);
	float3 F0 = 0.04;
	float3 color = float3(0.0f, 0.0f, 0.0f);
    float occlusion = 1.0f;
	
	Material material = materials[config.Index];
    material.Roughness = max(material.Roughness, 0.05f);
    
    if (config.HasMetallic) material.Metallic.a = material.Metallic.a * metallicTexture.Sample(materialSampler, texCoord.xy).b;
    if (config.HasBaseColor) material.BaseColor.rgb = material.BaseColor.rgb * InverseGammaCorrect(baseColorTexture.Sample(materialSampler, texCoord.xy).rgb);
    if (config.HasRoughness) material.Roughness.a = material.Roughness.a * roughnessTexture.Sample(materialSampler, texCoord.xy).g;
    if (config.HasOcclusion) occlusion = occlusionTexture.Sample(materialSampler, texCoord.xy).r;
    if (config.HasEmissive) material.Emissive.rgb = material.Emissive.rgb * InverseGammaCorrect(emissiveTexture.Sample(materialSampler, texCoord.xy).rgb);
	
	F0 = lerp(F0, material.BaseColor.xyz, material.Metallic.a);
	
    normal = normalize(getNormal(normal, texCoord.xy, tangent));
    
	[loop]
	for (uint index = 0; index < config.Lights; index++)
	{
		color = color + ComputePointLight(lights[index], material, position, normal, toEye, F0);
	}
	
	//ambient lighting
    if (config.HasIrradiance)
    {
        float3 N = normal;
        float3 V = toEye;
        float3 R = reflect(-V, N);
		
        float3 kS = FresnelSchlick(max(dot(N, V), 0.0f), F0);
        float3 kD = (1.0 - kS) * (1.0 - material.Metallic.a);
        float3 irradiance = irradianceMap.Sample(materialSampler, N).rgb;
        float3 diffuse = irradiance * material.BaseColor.rgb;
		
        color = color + kD * diffuse * occlusion;
    }
	
    color = color + material.Emissive.rgb;
	
    color = GammaCorrect(color);
	
	return float4(color, material.BaseColor.a);
}