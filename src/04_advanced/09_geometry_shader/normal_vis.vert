#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out VS_OUT{
    vec3 normal;
}vs_out;

uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = view * model* vec4(aPos, 1.0);
    vs_out.normal = transpose(inverse(mat3(view * model))) * aNormal;
}