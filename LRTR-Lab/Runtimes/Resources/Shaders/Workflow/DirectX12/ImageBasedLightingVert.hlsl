#pragma pack_matrix(row_major)

#define IBL_BUILD_ENVIRONMENT_MAP 0
#define IBL_BUILD_IRRADIANCE_MAP 1
#define IBL_BUILD_PRE_FILTERING_MAP 2
#define IBL_BUILD_PRE_COMPUTING_BRDF 3

struct View
{
    matrix View[8];
};

struct Config
{
    uint BuildType;
    uint Index;
    uint EnvironmentMapSize;
    float Roughenss;
};
struct Output
{
    float4 SVPosition : SV_POSITION;
    float3 Position : POSITION;
    float3 Texcoord : TEXCOORD;
};

ConstantBuffer<View> view : register(b0, space0);
Texture2D hdrTexture : register(t1, space0);
TextureCube environmentMap : register(t2, space0);

SamplerState textureSampler : register(s0, space1);
[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

Output main(float3 position : POSITION, float3 texcoord : TEXCOORD)
{
    Output result;

    if (config.BuildType == IBL_BUILD_PRE_COMPUTING_BRDF)
    {
        result.SVPosition = float4(position, 1.0f);
        result.Texcoord = texcoord;
        result.Position = position;
        
        return result;
    }
    
    result.SVPosition = mul(float4(position, 1.0f), view.View[config.Index]);    
    result.Position = position;
    result.Texcoord = texcoord;
    
    return result;
}
