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
Texture2D blurTexture : register(t1, space0);

SamplerState blurSampler : register(s0, space1);

[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

float4 main(float4 position : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
    const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

    float2 offset = 1.0 / float2(config.width, config.height);

    float4 result = blurTexture.Sample(blurSampler, texCoord) * weight[0];

    if (config.horizontal != 0) {
        for (int index = 1; index < 5; index++) {
            result = result + blurTexture.Sample(blurSampler, texCoord + float2(offset.x * index, 0)) * weight[index];
            result = result + blurTexture.Sample(blurSampler, texCoord - float2(offset.x * index, 0)) * weight[index];
        }
    }else {
        for (int index = 1; index < 5; index++) {
            result = result + blurTexture.Sample(blurSampler, texCoord + float2(0, offset.y * index)) * weight[index];
            result = result + blurTexture.Sample(blurSampler, texCoord - float2(0, offset.y * index)) * weight[index];
        }
    }

    return result;
}