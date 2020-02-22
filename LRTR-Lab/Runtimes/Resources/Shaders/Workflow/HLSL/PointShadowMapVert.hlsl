#pragma pack_matrix(row_major)

struct Transform
{
	matrix Transform;
};

struct View
{
    matrix View[8];
};

struct Config
{
    uint face;
    uint index;
    float farPlane;
    float positionX;
    float positionY;
    float positionZ;
};

struct Output
{
    float4 SVPosition : SV_POSITION;
    float3 Position : POSITION;
};

ConstantBuffer<View> view : register(b0, space0);
StructuredBuffer<Transform> transforms : register(t1, space0);

[[vk::push_constant]] ConstantBuffer<Config> config : register(b0, space2);

Output main(float3 position : POSITION)
{
    Output result;

    result.Position = mul(float4(position, 1.0f), transforms[config.index].Transform).xyz;
    result.SVPosition = mul(float4(result.Position, 1.0f), view.View[config.face]);

    return result;
}