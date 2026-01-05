#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform bool discardAlpha; // 是否丢弃透明片段(未启用GL_BLEND时可以丢弃透明片段来控制透明部分的渲染)

void main()
{    
    vec4 texColor = texture(texture1, TexCoords);
    if(discardAlpha && texColor.a < 0.1)
        discard; // 丢弃透明度低于0.1的片段
    else{
        FragColor = texture(texture1, TexCoords);; // 使用alpha通道,不丢弃，启用混合
    }
}