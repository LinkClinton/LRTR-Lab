#pragma pack_matrix(row_major)

struct AxisBufferData {
	matrix Transform;
	float4 Color;
};

StructuredBuffer<AxisBufferData> axisBuffer : register(t0);

float4 main(
	float4 position : SV_POSITION,
	uint identity : SV_INSTANCEID) : SV_TARGET
{
	return axisBuffer[identity].Color;
}