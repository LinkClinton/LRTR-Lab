#pragma pack_matrix(row_major)

struct AxisBufferData {
	matrix Transform;
};

struct Output {
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

struct View {
	matrix View;
};

StructuredBuffer<AxisBufferData> axisBuffer : register(t0);
ConstantBuffer<View> view : register(b1);

Output main(
	float3 position : POSITION,
	float4 color : COLOR,
	uint vertexId : SV_VERTEXID)
{
	Output result;

	result.Position = mul(float4(position, 1.0f), axisBuffer[vertexId / 2].Transform);
	result.Position = mul(result.Position, view.View);
	result.Color = color;

	return result;
}