#pragma pack_matrix(row_major)

struct View
{
	matrix View[4];
};

struct Samples
{
    float4 Samples[64];
};

struct Config
{
    uint SampleCount;
    float SampleRadius;
    float SampleBias;
    float NoiseScaleX;
    float NoiseScaleY;
    uint Width;
    uint Height;
    uint IsBlur;
};

ConstantBuffer<View> view : register(b0);
ConstantBuffer<Samples> samples : register(b1);

Texture2D noiseTexture : register(t2);
Texture2D positionAndOcclusionTexture : register(t3);
Texture2D viewSpacePositionTexture : register(t4);
Texture2D normalTexture : register(t5);
Texture2D ssaoTexture : register(t6);

SamplerState textureSampler : register(s0, space1);
SamplerState clampSampler : register(s1, space1);

[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

float OcclusionCalculation(float2 texCoord)
{
    float4 positionAndOcclusion = positionAndOcclusionTexture.Sample(textureSampler, texCoord);
    float3 viewSpacePosition = viewSpacePositionTexture.Sample(textureSampler, texCoord).xyz;
    float3 normal = normalize(normalTexture.Sample(textureSampler, texCoord).xyz);
    float3 noise = normalize(noiseTexture.Sample(textureSampler, texCoord * float2(config.NoiseScaleX, config.NoiseScaleY)).xyz);

    float3 N = normalize(mul(normal, (float3x3)view.View[3]));
    float3 T = normalize(noise - dot(noise, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);

    float occlusion = 0.0;
    float lengthViewSpacePosition = length(viewSpacePosition);

    for (uint index = 0; index < config.SampleCount; index++)
    {
        // transform the sample from tangent space to view space
        float3 sample = mul(samples.Samples[index].xyz, TBN);
        sample = viewSpacePosition + sample * config.SampleRadius;

        // transform the sample from view space to screen space
        float4 offset = float4(sample, 1.0);
        offset = mul(offset, view.View[1]);

        // perspective divide and transform them from [-1, 1] to [0, 1]
        offset.xyz = offset.xyz / offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = length(viewSpacePositionTexture.Sample(clampSampler, offset.xy).xyz);

        float rangeCheck = smoothstep(0.0, 1.0, config.SampleRadius / abs(lengthViewSpacePosition - sampleDepth));
        
        occlusion = occlusion + (sampleDepth >= (length(sample) + config.SampleBias) ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / config.SampleCount);

    return occlusion * positionAndOcclusion.w;
}

float BlurCalculation(float2 texCoord)
{
    float2 size = float2(1.0 / config.Width, 1.0 / config.Height);

    float result = 0;

    for (uint x = -2; x <= 2; x++){
        for (uint y = -2; y <= 2; y++)
        {
            float2 offset = float2(x, y) * size;
            result = result + ssaoTexture.Sample(textureSampler, texCoord + offset).r;
        }
    }

    result = result / (5.0 * 5.0);

    return result;
}

float main(
	float4 svPosition : SV_POSITION,
	float3 texCoord : TEXCOORD) : SV_TARGET
{
    if (config.IsBlur == 0) return OcclusionCalculation(texCoord.xy);
    else return BlurCalculation(texCoord.xy);
}