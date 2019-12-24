#pragma pack_matrix(row_major)

struct View
{
    matrix ViewWithoutTranslation;
};

struct Config
{
    uint IsSkyBox;
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

float4 main(float4 svPosition : SV_POSITION, float3 texcoord : TEXCOORD) : SV_TARGET
{
    return skyBox.Sample(textureSampler, texcoord);
}