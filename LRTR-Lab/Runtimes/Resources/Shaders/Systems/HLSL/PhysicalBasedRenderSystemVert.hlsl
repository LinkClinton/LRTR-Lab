#pragma pack_matrix(row_major)

struct View
{
	matrix View[4];
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

struct Output
{
	float4 SVPosition : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

ConstantBuffer<View> view : register(b1);

[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

Output main(
    float3 position : POSITION,
	float3 texCoord : TEXCOORD)
{
	Output result;
	
	result.SVPosition = mul(float4(position, 1.0f), view.View[1]);
	result.TexCoord = texCoord;

	return result;
}
