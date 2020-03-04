#pragma pack_matrix(row_major)

struct View
{
    matrix View[4];
};

struct Config
{
    uint horizontal;
    uint width;
    uint height;
};

struct Output
{
    float4 SVPosition : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

ConstantBuffer<View> view : register(b0, space0);

[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

Output main(float3 position : POSITION, float3 texCoord : TEXCOORD)
{
    Output result;

    result.SVPosition = mul(float4(position, 1.0f), view.View[0]);
    result.TexCoord = texCoord.xy;

    return result;
}