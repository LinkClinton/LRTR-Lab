#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform Color
{
	vec4 Color;
} color;


layout (location = 0) out vec4 outColor;

void main()
{
	outColor = color.Color;
}
