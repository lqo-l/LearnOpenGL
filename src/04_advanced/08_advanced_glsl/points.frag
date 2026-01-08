#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

uniform bool drawCircle;

void main()
{    
    if(drawCircle){
         // gl_PointCoord 是点图元内的归一化坐标 (0,0) 到 (1,1)
        // 中心是 (0.5, 0.5)
        
        vec2 coord = gl_PointCoord - vec2(0.5);  // 转换为中心坐标系
        
        // 计算距离中心的距离，如果大于0.5则丢弃（圆形裁剪）
        if(length(coord) > 0.5)
            discard;  // 丢弃圆外的片段
    }
    
    FragColor = vec4(1, 0.25, 0.5, 1); // 粒子颜色
}