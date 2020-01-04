#version 450
#extension GL_ARB_separate_shader_objects : enable

#define IBL_BUILD_ENVIRONMENT_MAP 0
#define IBL_BUILD_IRRADIANCE_MAP 1
#define IBL_BUILD_PRE_FILTERING_MAP 2
#define IBL_BUILD_PRE_COMPUTING_BRDF 3

#define PI 3.14159265359

layout (push_constant) uniform Config
{
    uint BuildType;
    uint Index;
    uint EnvironmentMapSize;
    float Roughenss;
} config;

layout (set = 0, binding = 1) uniform texture2D hdrTexture;
layout (set = 0, binding = 2) uniform textureCube environmentMap;

layout (set = 1, binding = 0) uniform sampler textureSampler;

vec2 SampleSphericalMap(vec3 position)
{
    vec2 uv = vec2(atan(position.z, position.x), asin(position.y));
    uv = uv * vec2(0.1591, 0.3183) + 0.5;
    
    return uv;
}

vec4 BuildEnvironmentMap(vec3 position)
{
    vec2 uv = SampleSphericalMap(normalize(position));
    
    return texture(sampler2D(hdrTexture, textureSampler), uv);
}

vec4 BuildIrradianceMap(vec3 position)
{
    vec3 normal = normalize(position);
    
    vec3 irradiance = vec3(0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up = cross(normal, right);
    
    float sampleDelta = 0.025;
    float nSamples = 0.0;
    
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            
            vec3 vectorSample = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
            
            

            irradiance = irradiance + texture(samplerCube(environmentMap, textureSampler), vectorSample).rgb * cos(theta) * sin(theta);
            nSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / nSamples);
    
    return vec4(irradiance, 1.0f);
}

float DistributionGGX(vec3 normal, vec3 halfVector, float roughness)
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

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverseVdC(i));
}

vec3 ImportanceSampleGGX(vec2 xi, vec3 normal, float roughenss)
{
    float a = roughenss * roughenss;
    
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);
    
    vec3 vectorSample = tangent * H.x + bitangent * H.y + normal * H.z;
    
    return normalize(vectorSample);
}

vec4 BuildPreFilteringMap(vec3 position)
{
    vec3 N = normalize(position);
    vec3 R = N;
    vec3 V = R;
    
    uint SampleCount = 1024;
    
    vec3 prefilteringColor = vec3(0);
    float weight = 0;
    
    for (uint index = 0; index < SampleCount; index++)
    {
        vec2 xi = Hammersley(index, SampleCount);
        vec3 H = ImportanceSampleGGX(xi, N, config.Roughenss);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        
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
            
            
            prefilteringColor = prefilteringColor + textureLod(samplerCube(environmentMap, textureSampler), L, mipLevel).rgb * NdotL;
            weight = weight + NdotL;
        }
    }

    prefilteringColor = prefilteringColor / weight;
    
    return vec4(prefilteringColor, 1.0);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;
    
    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;

    return numerator / denominator;
}

float GeometrySmith(vec3 normal, vec3 toEye, vec3 lightVector, float roughness)
{
    float normalDotEye = max(dot(normal, toEye), 0.0);
    float normalDotLight = max(dot(normal, lightVector), 0.0);
    float ggx2 = GeometrySchlickGGX(normalDotEye, roughness);
    float ggx1 = GeometrySchlickGGX(normalDotLight, roughness);

    return ggx1 * ggx2;
}

vec4 BuildPreComputingBRDF(float NdotV, float roughness)
{
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    float A = 0.0;
    float B = 0.0;
    
    vec3 N = vec3(0.0, 0.0, 1.0);
    
    uint SampleCount = 1024;
    
    for (uint index = 0; index < SampleCount; index++)
    {
        vec2 xi = Hammersley(index, SampleCount);
        vec3 H = ImportanceSampleGGX(xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        
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
    
    return vec4(A, B, 0, 0);
}

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 texcoord;

layout (location = 0) out vec4 color;

void main()
{
    if (config.BuildType == IBL_BUILD_ENVIRONMENT_MAP) 
        color = BuildEnvironmentMap(position);
    
    if (config.BuildType == IBL_BUILD_IRRADIANCE_MAP)
        color = BuildIrradianceMap(position);
    
    if (config.BuildType == IBL_BUILD_PRE_FILTERING_MAP)
        color = BuildPreFilteringMap(position);
    
    if (config.BuildType == IBL_BUILD_PRE_COMPUTING_BRDF)
        color = BuildPreComputingBRDF(texcoord.x, texcoord.y);
}