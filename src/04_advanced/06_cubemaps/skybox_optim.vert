#version 430 core
layout (location = 0) in vec3 aPos;


out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;


void main()
{
    TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
    // gl_Position = gl_Position.xyww; // 设置深度值为1.0，让天空盒始终在最远处
    gl_Position.z = gl_Position.w; // 设置深度值为1.0，让天空盒始终在最远处
}