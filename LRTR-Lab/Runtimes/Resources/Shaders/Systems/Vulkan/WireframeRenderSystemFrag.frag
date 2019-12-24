#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform Config
{
	vec4 Color;
	uint Index;
} config;


layout (location = 0) out vec4 outColor;

void main()
{
	outColor = config.Color;
}
