#version 430 core
layout (location = 0) in vec3 aPos;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float pointSize;
uniform float distFactor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    gl_PointSize = pointSize * (gl_Position.z * distFactor + 1.0);
}