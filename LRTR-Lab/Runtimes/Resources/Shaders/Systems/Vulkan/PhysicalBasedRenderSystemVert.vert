#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 2) buffer Transforms    
{
    mat4 Transform[];
} transforms;

layout (set = 0, binding = 3) uniform View
{
    mat4 View;
} view;

layout (push_constant) uniform Config
{
    uint HasEnvironmentLight;
    uint HasBaseColor;
    uint HasRoughness;
    uint HasOcclusion;
    uint HasNormalMap;
    uint HasMetallic;
    uint HasEmissive;
    float EyePositionX;
    float EyePositionY;
    float EyePositionZ;
    uint MipLevels;
    uint Lights;
    uint Index;
} config;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTexcoord;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outTexcoord;
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outNormal;

void main()
{
    outPosition = (transforms.Transform[config.Index] * vec4(inPosition, 1.0f)).xyz;
    outTexcoord = inTexcoord;
    outTangent = mat3(transforms.Transform[config.Index]) * inTangent;
    outNormal = mat3(transforms.Transform[config.Index]) * inNormal;

    gl_Position = view.View * vec4(outPosition, 1.0f);
}