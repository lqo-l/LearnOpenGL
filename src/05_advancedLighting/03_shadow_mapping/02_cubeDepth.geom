#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out; // 一个三角形在6个面渲染，共18个顶点

uniform mat4 shadowMatrices[6]; // 6个面对应的视图投影矩阵*视图矩阵

out vec4 fragWorldPos;


void main(){
    for(int face = 0; face < 6; face++){
        gl_Layer = face; // 指定渲染到立方体贴图的第face层. built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; i++){
            fragWorldPos = gl_in[i].gl_Position; // 世界空间位置传递给片段着色器
            gl_Position = shadowMatrices[face] * gl_in[i].gl_Position; // 投影到立方体贴图的6个面上
            EmitVertex();
        }
        EndPrimitive();
    }
}