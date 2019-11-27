#pragma pack_matrix(row_major)

struct Config {
	float4 Color;
	uint Index;
};

ConstantBuffer<Config> config : register(b2);

float4 main(
	float4 position : SV_POSITION) : SV_TARGET
{ 
	return config.Color;
}