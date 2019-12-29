#pragma pack_matrix(row_major)

struct View
{
    matrix ViewWithoutTranslation;
};

struct Config
{
    uint IsSkyBox;
    uint IsHDR;
};

struct Output
{
    float4 SVPosition : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

ConstantBuffer<View> view : register(b0);
ConstantBuffer<Config> config : register(b3);

SamplerState textureSampler : register(s2);

TextureCube skyBox : register(t1);

Output main(float3 position : POSITION)
{
    Output result;
    
    result.SVPosition = mul(float4(position, 1.0f), view.ViewWithoutTranslation);
    result.SVPosition = result.SVPosition.xyww;
    result.TexCoord = position;
    
    return result;
}