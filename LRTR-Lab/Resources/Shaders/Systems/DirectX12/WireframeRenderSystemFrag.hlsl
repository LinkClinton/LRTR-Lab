#pragma pack_matrix(row_major)

struct Color {
	float4 Color;
};

ConstantBuffer<Color> color : register(b2);

float4 main(
	float4 position : SV_POSITION) : SV_TARGET
{ 
	return color.Color;
}