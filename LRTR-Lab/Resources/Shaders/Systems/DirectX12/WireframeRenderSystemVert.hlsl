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

StructuredBuffer<MeshBufferData> meshBuffer : register(t0);
ConstantBuffer<View> view : register(b1);

Output main(
	float3 position : POSITION,
	uint identity : SV_INSTANCEID)
{
	Output result;

	result.Position = mul(float4(position, 1.0f), meshBuffer[identity].Transform);
	result.Position = mul(result.Position, view.View);
	
	return result;
}