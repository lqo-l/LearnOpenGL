#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;


void main()
{    
    FragColor = texture(screenTexture, TexCoords); // 使用alpha通道,不丢弃，启用混合
}