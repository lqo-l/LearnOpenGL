#version 430 core
layout (location = 0) in vec3 aPos;

/// opengl 4.2 之后支持使用 binding 指定绑定点
// layout(std140, binding = 1) uniform Matrices{
//     mat4 view;
//     mat4 projection;
// };

// 从 opengl 4.2 之前的版本写法,在c++程序中指定绑定点
layout(std140) uniform Matrices{
    mat4 view;
    mat4 projection;
};

uniform mat4 model;


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}