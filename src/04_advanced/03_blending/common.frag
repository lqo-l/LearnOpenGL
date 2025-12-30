#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a < 0.1)
        discard; // 丢弃透明度低于0.1的片段

    FragColor = texColor; // 使用alpha通道
}