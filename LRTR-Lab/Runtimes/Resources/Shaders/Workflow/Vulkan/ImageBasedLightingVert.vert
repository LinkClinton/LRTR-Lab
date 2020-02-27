#version 450
#extension GL_ARB_separate_shader_objects : enable

#define IBL_BUILD_ENVIRONMENT_MAP 0
#define IBL_BUILD_IRRADIANCE_MAP 1
#define IBL_BUILD_PRE_FILTERING_MAP 2
#define IBL_BUILD_PRE_COMPUTING_BRDF 3

layout (set = 0, binding = 0) uniform View
{
    mat4 View[8];
} views;

layout (push_constant) uniform Config
{
    uint BuildType;
    uint Index;
    uint EnvironmentMapSize;
    float Roughenss;
} config;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTexcoord;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outTexcoord;

void main()
{
    if (config.BuildType == IBL_BUILD_PRE_COMPUTING_BRDF)
        gl_Position = vec4(inPosition, 1.0f);
    else {
        gl_Position = views.View[config.Index] * vec4(inPosition, 1.0f);
        gl_Position.y = -gl_Position.y;
    }

    outPosition = inPosition;
    outTexcoord = inTexcoord;
}