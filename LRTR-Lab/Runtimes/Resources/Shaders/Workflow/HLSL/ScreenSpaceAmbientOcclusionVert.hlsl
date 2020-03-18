#pragma pack_matrix(row_major)

struct View
{
	matrix View[4];
};

struct Output
{
	float4 SVPosition : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

ConstantBuffer<View> view : register(b0);

Output main(
    float3 position : POSITION,
	float3 texCoord : TEXCOORD)
{
	Output result;
	
	result.SVPosition = mul(float4(position, 1.0f), view.View[0]);
	result.TexCoord = texCoord;

	return result;
}
