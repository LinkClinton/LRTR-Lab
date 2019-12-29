#pragma pack_matrix(row_major)

struct View
{
    matrix View[8];
};

struct Config
{
    uint Index;
};

struct Output
{
    float4 SVPosition : SV_POSITION;
    float3 Position : POSITION;
};

ConstantBuffer<View> view : register(b0, space0);
Texture2D hdrTexture : register(t1, space0);

SamplerState textureSampler : register(s0, space1);
ConstantBuffer<Config> config : register(b0, space2);

Output main(float3 position : POSITION)
{
    Output result;
    
    result.Position = position;
    result.SVPosition = mul(float4(position, 1.0f), view.View[config.Index]);
    
    return result;
}
