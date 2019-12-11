#pragma pack_matrix(row_major)

#define PI 3.14159265359

struct Material
{
	float4 BaseColor;
	float4 Roughness;
	float4 Metallic;
};

struct Light
{
	float3 Position;
	float3 Intensity;
};

struct Config
{
	uint HasBaseColor;
	uint HasRoughness;
	uint HasOcclusion;
	uint HasNormalMap;
	uint HasMetallic;
	float3 EyePosition;
	uint Lights;
	uint Index;
};

float3 mix(float3 x, float3 y, float3 a)
{
	return x * (1.0 - a) + y * a;
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
	return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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

	return (kD * material.BaseColor.xyz / PI + numerator / max(denominator, 0.001)) * radiance;
}

float3 ComputePointLight(Light light, Material material, float3 position, float3 normal, float3 toEye, float3 F0)
{
	float3 lightVector = normalize(light.Position - position);
	
	float ndotl = max(dot(lightVector, normal), 0.0f);
	float distance = length(light.Position - position);
	float attenuation = 1.0f / (distance * distance);
	
	return CookTorranceBRDF(material, light.Intensity * attenuation * ndotl, lightVector, normal, toEye, F0);
}

StructuredBuffer<Material> materials : register(t0);
StructuredBuffer<Light> lights : register(t1);
ConstantBuffer<Config> config : register(b9);

float4 main(
	float4 svPosition : SV_POSITION,
	float3 position : POSITION,
	float3 texCoord : TEXCOORD,
	float3 tangent : TANGENT,
	float3 normal : NORMAL) : SV_TARGET
{
	float3 toEye = normalize(config.EyePosition - position);
	float3 F0 = 0.04;
	float3 color = float3(0.0f, 0.0f, 0.0f);

	Material material = materials[config.Index];
	
	F0 = lerp(F0, material.BaseColor.xyz, material.Metallic.a);
	
	normal = normalize(normal);
    material.Roughness = max(material.Roughness, 0.05f);
    
	[loop]
	for (uint index = 0; index < config.Lights; index++)
	{
		color = color + ComputePointLight(lights[index], material, position, normal, toEye, F0);
	}
	
    color = color + 0.1f * material.BaseColor.xyz;
	
	color = color / (color + 1.0f);
	color = pow(color, 1.0 / 2.2);
	
	return float4(color, material.BaseColor.a);
}