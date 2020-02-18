#pragma pack_matrix(row_major)

#define IBL_BUILD_ENVIRONMENT_MAP 0
#define IBL_BUILD_IRRADIANCE_MAP 1
#define IBL_BUILD_PRE_FILTERING_MAP 2
#define IBL_BUILD_PRE_COMPUTING_BRDF 3

#define PI 3.14159265359

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

float2 SampleSphericalMap(float3 position)
{
    float2 uv = float2(atan2(position.z, position.x), asin(position.y));
    uv = uv * float2(0.1591, 0.3183) + 0.5;
    
    return uv;
}

float4 BuildEnvironmentMap(float3 position)
{
    float2 uv = SampleSphericalMap(normalize(position));
    
    return hdrTexture.Sample(textureSampler, uv);
}

float4 BuildIrradianceMap(float3 position)
{
    float3 normal = normalize(position);
    
    float3 irradiance = 0;

    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = cross(up, normal);
    up = cross(normal, right);
    
    float sampleDelta = 0.025;
    float nSamples = 0.0;
    
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            
            float3 vectorSample = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
            
            irradiance = irradiance + environmentMap.Sample(textureSampler, vectorSample).rgb * cos(theta) * sin(theta);
            nSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / nSamples);
    
    return float4(irradiance, 1.0f);
}

float DistributionGGX(float3 normal, float3 halfVector, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float normalDotHalf = max(dot(normal, halfVector), 0.0);
    float normalDotHalf2 = normalDotHalf * normalDotHalf;

    float numerator = a2;
    float denominator = (normalDotHalf2 * (a2 - 1.0) + 1.0);

    denominator = PI * denominator * denominator;

    return numerator / denominator;
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverseVdC(i));
}

float3 ImportanceSampleGGX(float2 xi, float3 normal, float roughenss)
{
    float a = roughenss * roughenss;
    
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    float3 H = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    
    float3 up = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);
    
    float3 vectorSample = tangent * H.x + bitangent * H.y + normal * H.z;
    
    return normalize(vectorSample);
}

float4 BuildPreFilteringMap(float3 position)
{
    float3 N = normalize(position);
    float3 R = N;
    float3 V = R;
    
    uint SampleCount = 1024;
    
    float3 prefilteringColor = 0;
    float weight = 0;
    
    for (uint index = 0; index < SampleCount; index++)
    {
        float2 xi = Hammersley(index, SampleCount);
        float3 H = ImportanceSampleGGX(xi, N, config.Roughenss);
        float3 L = normalize(2.0 * dot(V, H) * H - V);
        
        float NdotL = max(dot(N, L), 0.0);
        
        if (NdotL > 0.0)
        {
            float D = DistributionGGX(N, H, config.Roughenss);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;
            
            float sampleTexel = 4.0 * PI / (6.0 * config.EnvironmentMapSize * config.EnvironmentMapSize);
            float sampleSample = 1.0 / (float(SampleCount) * pdf + 0.0001);

            float mipLevel = config.Roughenss == 0.0 ? 0.0 : 0.5 * log2(sampleSample / sampleTexel);
            
            prefilteringColor = prefilteringColor + environmentMap.SampleLevel(textureSampler, L, mipLevel).rgb * NdotL;
            weight = weight + NdotL;
        }
    }

    prefilteringColor = prefilteringColor / weight;
    
    return float4(prefilteringColor, 1.0);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;
    
    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;

    return numerator / denominator;
}

float GeometrySmith(float3 normal, float3 toEye, float3 lightVector, float roughness)
{
    float normalDotEye = max(dot(normal, toEye), 0.0);
    float normalDotLight = max(dot(normal, lightVector), 0.0);
    float ggx2 = GeometrySchlickGGX(normalDotEye, roughness);
    float ggx1 = GeometrySchlickGGX(normalDotLight, roughness);

    return ggx1 * ggx2;
}

float4 BuildPreComputingBRDF(float NdotV, float roughness)
{
    float3 V = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    float A = 0.0;
    float B = 0.0;
    
    float3 N = float3(0.0, 0.0, 1.0);
    
    uint SampleCount = 1024;
    
    for (uint index = 0; index < SampleCount; index++)
    {
        float2 xi = Hammersley(index, SampleCount);
        float3 H = ImportanceSampleGGX(xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);
        
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        
        if (NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float GVis = (G * VdotH) / (NdotH * NdotV);
            float F = pow(1.0 - VdotH, 5.0);
            
            A = A + (1.0 - F) * GVis;
            B = B + F * GVis;
        }
    }

    A = A / float(SampleCount);
    B = B / float(SampleCount);
    
    return float4(A, B, 0, 0);
}

float4 main(float4 svPosition : SV_POSITION, float3 position : POSITION, float3 texcoord : TEXCOORD) : SV_TARGET
{
    if (config.BuildType == IBL_BUILD_ENVIRONMENT_MAP) 
        return BuildEnvironmentMap(position);
    
    if (config.BuildType == IBL_BUILD_IRRADIANCE_MAP)
        return BuildIrradianceMap(position);
    
    if (config.BuildType == IBL_BUILD_PRE_FILTERING_MAP)
        return BuildPreFilteringMap(position);
    
    if (config.BuildType == IBL_BUILD_PRE_COMPUTING_BRDF)
        return BuildPreComputingBRDF(texcoord.x, texcoord.y);
    
    return float4(1, 0, 0, 1);
}