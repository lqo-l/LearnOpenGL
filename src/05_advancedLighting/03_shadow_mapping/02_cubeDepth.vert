#version 430 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;

void main(){
    gl_Position = model * vec4(aPos, 1.0); // 变换到世界空间，送到几何着色器
}