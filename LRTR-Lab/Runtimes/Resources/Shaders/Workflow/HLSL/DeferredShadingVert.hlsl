#pragma pack_matrix(row_major)

struct Transform
{
	matrix Transform;
};

struct View
{
	matrix View[4];
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

struct Output
{
	float4 SVPosition : SV_POSITION;
	float3 Position : POSITION;
	float3 TexCoord : TEXCOORD;
	float3 Tangent : TANGENT;
	float3 Normal : NORMAL;
};

StructuredBuffer<Transform> transforms : register(t1);
ConstantBuffer<View> view : register(b2);

[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

Output main(
    float3 position : POSITION,
	float3 texCoord : TEXCOORD,
    float3 tangent : TANGENT,
    float3 normal : NORMAL)
{
	Output result;
	
	result.Position = mul(float4(position, 1.0f), transforms[config.Index].Transform).xyz;
	result.SVPosition = mul(float4(result.Position, 1.0f), view.View[0]);
	result.Normal = mul(normal, (float3x3)transforms[config.Index].Transform); //no scale transform
	result.Tangent = mul(tangent, (float3x3)transforms[config.Index].Transform); //no scale transform
	result.TexCoord = texCoord;

	return result;
}