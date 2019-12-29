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

float2 SampleSphericalMap(float3 position)
{
    float2 uv = float2(atan2(position.z, position.x), asin(position.y));
    uv = uv * float2(0.1591, 0.3183) + 0.5;
    
    return uv;
}

float4 main(float4 svPosition : SV_POSITION, float3 position : POSITION) : SV_TARGET
{
    float2 uv = SampleSphericalMap(normalize(position));
    
    return hdrTexture.Sample(textureSampler, uv);
}