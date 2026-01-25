#version 430 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 offset;


out vec3 Color;

void main()
{
    Color = aColor;
    gl_Position = vec4(aPos + offset, 0.0, 1.0);
}