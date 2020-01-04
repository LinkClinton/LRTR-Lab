#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform Config
{
    uint IsSkyBox;
    uint IsHDR;
} config;

layout (set = 0, binding = 2) uniform sampler textureSampler;
layout (set = 0, binding = 1) uniform textureCube skyBox;

layout (location = 0) in vec3 texcoord;
layout (location = 0) out vec4 outColor;

float GammaCorrect(float value)
{
    if (value <= 0.0031308f)
        return 12.92f * value;
	
    return 1.055f * pow(value, 1.f / 2.4f) - 0.055f;
}

vec3 GammaCorrect(vec3 value)
{
    return vec3(
		GammaCorrect(value.x),
		GammaCorrect(value.y),
		GammaCorrect(value.z));
}

void main()
{
    vec3 color = texture(samplerCube(skyBox, textureSampler), texcoord).rgb;

    if (config.IsHDR != 0) color = GammaCorrect(color);

    outColor = vec4(color, 1.0f);
}