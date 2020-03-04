#pragma pack_matrix(row_major)

struct View
{
    matrix ViewWithoutTranslation;
    matrix ViewForBlurBlend;
    matrix Unused0;
    matrix Unused1;
};

struct Config
{
    uint IsSkyBox;
    uint IsBlur;
    uint IsHDR;
};

struct Output
{
    float4 SVPosition : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

ConstantBuffer<View> view : register(b0);
[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

SamplerState textureSampler : register(s0, space1);

TextureCube skyBox : register(t1);

Output main(float3 position : POSITION, float3 texCoord : TEXCOORD)
{
    Output result;
    
    if (config.IsSkyBox != 0) {
        result.SVPosition = mul(float4(position, 1.0f), view.ViewWithoutTranslation);
        result.SVPosition = result.SVPosition.xyww;
        
        // because we set the x-y plane as horizon, we we need rotate the sky box
        result.TexCoord = float3(position.x, position.z, -position.y);

        return result;
    }

    if (config.IsBlur != 0) {
        result.SVPosition = mul(float4(position, 1.0f), view.ViewForBlurBlend);
        result.TexCoord = texCoord;

        return result; 
    }

    return result;
}