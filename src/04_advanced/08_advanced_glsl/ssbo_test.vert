#version 430 core
layout (location = 0) in vec3 aPos;

// 测试读；测试写；
layout(std430, binding=1) buffer testBlock{
    mat4 view;
    mat4 projection;
    vec3 color;
    vec3 write_color_in_shader; // 测试在shader中写入与color相同的值
};

uniform mat4 model;

// 测试接口块
out VS_OUT{
    vec3 fragColor;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.fragColor = color;

    // 测试写
    write_color_in_shader = color; 
}