#pragma pack_matrix(row_major)

struct MeshBufferData {
	matrix Transform;
};

struct Output {
	float4 Position : SV_POSITION;
};

struct View {
	matrix View;
};

struct Config {
	float4 Color;
	uint Index;
};

StructuredBuffer<MeshBufferData> meshBuffer : register(t0);
ConstantBuffer<View> view : register(b1);
[[vk::push_constant]] ConstantBuffer<Config> config : register(b2);

Output main(
	float3 position : POSITION)
{
	Output result;

	result.Position = mul(float4(position, 1.0f), meshBuffer[config.Index].Transform);
	result.Position = mul(result.Position, view.View);
	
	return result;
}