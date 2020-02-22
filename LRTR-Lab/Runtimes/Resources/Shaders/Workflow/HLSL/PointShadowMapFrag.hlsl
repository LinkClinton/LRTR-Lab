#pragma pack_matrix(row_major)

struct Config
{
    uint face;
    uint index;
    float farPlane;
    float positionX;
    float positionY;
    float positionZ;
};

[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

float main(float4 svPosition : SV_POSITION, float3 position : POSITION) : SV_DEPTH
{
    float distance = length(position - float3(config.positionX, config.positionY, config.positionZ));

    return distance / config.farPlane;
}