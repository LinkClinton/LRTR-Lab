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

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
    gl_Position = (transforms.Transform[gl_VertexIndex / 2] * vec4(inPosition, 1.0));
    gl_Position = (view.View * gl_Position);
    gl_Position.y = -gl_Position.y;

	outColor = inColor;
}