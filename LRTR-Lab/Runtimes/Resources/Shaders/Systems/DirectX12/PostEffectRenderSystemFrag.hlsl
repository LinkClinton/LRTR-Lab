#pragma pack_matrix(row_major)

struct View
{
    matrix ViewWithoutTranslation;
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
Texture2D blurTexture : register(t2);

float GammaCorrect(float value)
{
    if (value <= 0.0031308f)
        return 12.92f * value;
	
    return 1.055f * pow(value, 1.f / 2.4f) - 0.055f;
}

float3 GammaCorrect(float3 value)
{
    return float3(
		GammaCorrect(value.x),
		GammaCorrect(value.y),
		GammaCorrect(value.z));
}

float4 main(float4 svPosition : SV_POSITION, float3 texcoord : TEXCOORD) : SV_TARGET
{
    if (config.IsSkyBox != 0) {
        float3 color = skyBox.Sample(textureSampler, texcoord).rgb;
    
        if (config.IsHDR != 0) return float4(GammaCorrect(color), 1.0f);
        else return float4(color, 1.0f);
    }

    if (config.IsBlur != 0) {
        return blurTexture.Sample(textureSampler, texcoord.xy);
    }

    // only for error
    return float4(1, 0, 0, 0);
}