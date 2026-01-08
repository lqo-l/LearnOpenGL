#version 430 core

in vec3 TexCoords; // 立方体贴图纹理坐标

out vec4 FragColor;

uniform samplerCube skybox; // 立方体贴图采样器


void main()
{    
    FragColor = texture(skybox, TexCoords); 
}