#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 outlineColor;

void main()
{    
    FragColor = vec4(outlineColor, 1.0);
}