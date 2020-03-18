#pragma pack_matrix(row_major)

struct Material
{
	float4 BaseColor;
	float4 Roughness;
	float4 Metallic;
    float4 Emissive;
};

struct Config
{
    uint HasBaseColor;
    uint HasRoughness;
    uint HasOcclusion;
    uint HasNormalMap;
    uint HasMetallic;
    uint HasEmissive;
	uint HasBlurred;
    uint Index;
};

StructuredBuffer<Material> materials : register(t0);

Texture2D metallicTexture : register(t3);
Texture2D baseColorTexture : register(t4);
Texture2D roughnessTexture : register(t5);
Texture2D occlusionTexture : register(t6);
Texture2D normalMapTexture : register(t7);
Texture2D emissiveTexture : register(t8);

SamplerState materialSampler : register(s0, space1);
[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

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

struct Output {
    float4 BaseColorAndRoughness : SV_TARGET0;
    float4 PositionAndOcclusion  : SV_TARGET1;
    float4 EmissiveAndMetallic   : SV_TARGET2;
    float4 ViewSpacePosition     : SV_TARGET3;
    float4 Normal                : SV_TARGET4;
};

Output main(
	float4 svPosition : SV_POSITION,
    float3 vPosition : POSITION0,
	float3 position : POSITION1,
	float3 texCoord : TEXCOORD,
	float3 tangent : TANGENT,
	float3 normal : NORMAL)
{
    Output result;

	Material material = materials[config.Index];
    material.Roughness = max(material.Roughness, 0.05f);

    float occlusion = 1.0f;
	
    if (config.HasMetallic) material.Metallic.a = material.Metallic.a * metallicTexture.Sample(materialSampler, texCoord.xy).b;
    if (config.HasBaseColor) material.BaseColor.rgb = material.BaseColor.rgb * InverseGammaCorrect(baseColorTexture.Sample(materialSampler, texCoord.xy).rgb);
    if (config.HasRoughness) material.Roughness.a = material.Roughness.a * roughnessTexture.Sample(materialSampler, texCoord.xy).g;
    if (config.HasOcclusion) occlusion = occlusionTexture.Sample(materialSampler, texCoord.xy).r;
    if (config.HasEmissive) material.Emissive.rgb = material.Emissive.rgb * InverseGammaCorrect(emissiveTexture.Sample(materialSampler, texCoord.xy).rgb);
	
    normal = normalize(getNormal(normal, texCoord.xy, tangent));

    result.BaseColorAndRoughness = float4(material.BaseColor.rgb, material.Roughness.a);
    result.PositionAndOcclusion = float4(position, occlusion);
    result.EmissiveAndMetallic = float4(material.Emissive.rgb, material.Metallic.a);
    result.ViewSpacePosition = float4(vPosition, occlusion);
    result.Normal = float4(normal, config.HasBlurred != 0 ? 1.0f : 0.0f);

    return result;
}