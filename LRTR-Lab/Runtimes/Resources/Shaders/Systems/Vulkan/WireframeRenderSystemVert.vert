#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) buffer Transform
{
    mat4 Transform[];
} transforms;

layout (set = 0, binding = 1) uniform View
{
    mat4 View;
} view;

layout (push_constant) uniform Config
{
	vec4 Color;
	uint Index;
} config;

layout (location = 0) in vec3 inPosition;

void main()
{
    gl_Position = (transforms.Transform[config.Index] * vec4(inPosition, 1.0));
    gl_Position = (view.View * gl_Position);
}