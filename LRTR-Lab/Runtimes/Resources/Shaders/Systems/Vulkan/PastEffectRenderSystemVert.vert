#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform View
{
    mat4 ViewWithoutTranslation;
} view;

layout (push_constant) uniform Config
{
    uint IsSkyBox;
    uint IsHDR;
} config;

layout (location = 0) in vec3 position;
layout (location = 0) out vec3 texcoord;

void main()
{
    gl_Position = vec4(position, 1.0f);
    gl_Position = (view.ViewWithoutTranslation * gl_Position);
    gl_Position = gl_Position.xyww;
    
    texcoord = position;
}