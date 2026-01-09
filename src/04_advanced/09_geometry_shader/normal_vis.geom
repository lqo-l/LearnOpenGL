#version 430 core
layout (triangles) in; 
layout (line_strip, max_vertices = 6) out; 


in VS_OUT{
    vec3 normal;
}gs_in[];

uniform float normalLength;
uniform mat4 projection;


void main(){
    // 传入的坐标和法线是视图空间的，沿法线方向移动顶点

    for(int i = 0; i<3; i++){
        // 绘制法线起点
        gl_Position = projection * gl_in[i].gl_Position;
        EmitVertex();

        // 绘制法线终点
        vec3 normal = normalize(gs_in[i].normal);
        vec4 normal_end_pos = gl_in[i].gl_Position + vec4(normal * normalLength, 0.0);
        gl_Position = projection * normal_end_pos;
        EmitVertex();
        EndPrimitive();
    }

    
}