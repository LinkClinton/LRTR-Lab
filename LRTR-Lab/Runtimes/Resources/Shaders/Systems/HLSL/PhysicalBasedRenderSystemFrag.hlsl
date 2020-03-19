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
	float4 Position;
	float4 Intensity;

	float FarPlane;
	uint Index;
	uint Type;
	uint Unused;
};

struct Config
{
    uint HasEnvironmentLight;
    float EyePositionX;
	float EyePositionY;
    float EyePositionZ;
    uint MipLevels;
	uint Lights;
};

float3 mix(float3 x, float3 y, float3 a)
{
	return x * (1.0 - a) + y * a;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(1.0 - roughness, F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
	float3 lightVector = normalize(light.Position.xyz - position);
	
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

StructuredBuffer<Light> lights : register(t0);

Texture2D baseColorAndRoughnessTexture : register(t2);
Texture2D positionAndOcclusionTexture : register(t3);
Texture2D emissiveAndMetallicTexture : register(t4);
Texture2D normalAndBlurTexture : register(t5);
Texture2D depthTexture : register(t6);
Texture2D ssaoTexture : register(t7);
TextureCube irradianceMap : register(t8);
TextureCube preFilteringMap : register(t9);
Texture2D preComputingBRDF : register(t10);
TextureCubeArray pointShadowMaps : register(t11);

SamplerState textureSampler : register(s0, space1);
[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

float ShadowCalculation(float3 position, float viewDistance, uint index)
{
	const float3 gridSamplingDisk[20] = {
		float3( 1,  1,  1), float3( 1, -1,  1), float3(-1, -1,  1), float3(-1,  1,  1), 
		float3( 1,  1, -1), float3( 1, -1, -1), float3(-1, -1, -1), float3(-1,  1, -1),
		float3( 1,  1,  0), float3( 1, -1,  0), float3(-1, -1,  0), float3(-1,  1,  0),
		float3( 1,  0,  1), float3(-1,  0,  1), float3( 1,  0, -1), float3(-1,  0, -1),
		float3( 0,  1,  1), float3( 0, -1,  1), float3( 0, -1, -1), float3( 0,  1, -1)
	};

	if (lights[index].Index == 0) return 0;

	float3 lightPosition = lights[index].Position.xyz;
	float3 fragToLight = position - lightPosition;
	float diskRadius = (1.0 + (viewDistance / lights[index].FarPlane)) / 25.0;
	float current = length(fragToLight);
	float bias = 0.15;

	float shadow = 0.0;
	uint samples = 20;

	float mapIndex = lights[index].Index - 1;

	for (uint i = 0; i < samples; i++)
	{
		float closest = lights[index].FarPlane * 
			pointShadowMaps.Sample(textureSampler, float4(fragToLight + gridSamplingDisk[i] * diskRadius, mapIndex)).r;
		
		if (current - bias > closest) shadow = shadow + 1.0;
	}

	return shadow / float(samples);
}

struct Output {
	float4 Color0 : SV_TARGET0;
	float4 Color1 : SV_TARGET1;
    float Depth : SV_DEPTH;
};

Output main(
	float4 svPosition : SV_POSITION,
	float3 texCoord : TEXCOORD)
{
    float4 baseColorAndRoughness = baseColorAndRoughnessTexture.Sample(textureSampler, texCoord.xy);
    float4 positionAndOcclusion = positionAndOcclusionTexture.Sample(textureSampler, texCoord.xy);
    float4 emissiveAndMetallic = emissiveAndMetallicTexture.Sample(textureSampler, texCoord.xy);
    float4 normalAndBlur = normalAndBlurTexture.Sample(textureSampler, texCoord.xy);
    float occlusion = ssaoTexture.Sample(textureSampler, texCoord.xy).r;

    Material material;

    material.BaseColor.xyz = baseColorAndRoughness.xyz;
    material.Roughness.a = baseColorAndRoughness.a;
    material.Metallic.a = emissiveAndMetallic.a;
    material.Emissive.xyz = emissiveAndMetallic.xyz;

    float3 position = positionAndOcclusion.xyz;
    float3 normal = normalAndBlur.xyz;

    float3 toEye = normalize(float3(config.EyePositionX, config.EyePositionY, config.EyePositionZ) - position);
	float3 F0 = 0.04;
	float3 color = float3(0.0f, 0.0f, 0.0f);
    float viewDistance = length(float3(config.EyePositionX, config.EyePositionY, config.EyePositionZ) - position);
    
	F0 = lerp(F0, material.BaseColor.xyz, material.Metallic.a);
    
	[loop]
	for (uint index = 0; index < config.Lights; index++)
	{
		color = color + ComputePointLight(lights[index], material, position, normal, toEye, F0) * (1.0 - ShadowCalculation(position, viewDistance, index));
	}
	
	//ambient lighting with environment map
    if (config.HasEnvironmentLight)
    {
        float3 N = normal;
        float3 V = toEye;
        float3 R = reflect(-V, N);
		
        float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, material.Roughness.a);
		
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - material.Metallic.a);
		
        float3 irradiance = irradianceMap.Sample(textureSampler, N).rgb;
        float3 diffuse = irradiance * material.BaseColor.rgb;
		
        float MaxLod = config.MipLevels - 1.0;
        float NdotV = max(dot(N, V), 0.0);

        float3 preFilteringColor = preFilteringMap.SampleLevel(textureSampler, R, material.Roughness.a * MaxLod).rgb;
        float2 brdf = preComputingBRDF.Sample(textureSampler, float2(NdotV, material.Roughness.a)).rg;
        float3 specular = preFilteringColor * (F * brdf.x + brdf.y);
		
        color = color + (kD * diffuse + specular) * occlusion;
	}
	else {
		//if we do not have environment map, we will use default environment light
		color = color + material.BaseColor.xyz * 0.03 * occlusion;
	}
	
    //color = color + material.Emissive.rgb;
	
    //color = GammaCorrect(color);
	
	Output result;

	result.Color0 = float4(color, 1.f);
	
	if (normalAndBlur.a != 0.0f) result.Color1 = float4(color, 1.f);
	else result.Color1 = float4(0, 0, 0, 0);

    result.Depth = depthTexture.Sample(textureSampler, texCoord.xy).r;

	return result;
}