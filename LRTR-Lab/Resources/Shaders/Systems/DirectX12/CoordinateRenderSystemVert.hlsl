#pragma pack_matrix(row_major)

struct AxisBufferData {
	matrix Transform;
	float4 Color;
};

struct Output {
	float4 Position : SV_POSITION;
	uint Identity : SV_INSTANCEID;
};

struct View {
	matrix View;
};

StructuredBuffer<AxisBufferData> axisBuffer : register(t0);
ConstantBuffer<View> view : register(b1);

Output main(
	float3 position : POSITION,
	uint identity : SV_INSTANCEID)
{
	Output result;

	result.Position = mul(float4(position, 1.0f), axisBuffer[identity].Transform);
	result.Position = mul(result.Position, view.View);
	result.Identity = identity;

	return result;
}