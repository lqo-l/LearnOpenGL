#version 430 core
layout (points) in; // 声明从顶点着色器接收的图元类型为points
layout (triangle_strip, max_vertices = 5) out; 

// 使用内置的gl_PerVertex结构体来访问顶点数据，不需要定义，直接用gl_in访问即可
// in gl_PerVertex{ 
//     vec4 gl_Position;
//     float gl_PointSize;
//     float gl_ClipDistance[];
// } gl_in[]; // 数组，包含一个图元的所有顶点

in VS_OUT{
    vec3 color;
}gs_in[];

out vec3 fColor; // 传递给片段着色器的颜色

void main(){
    // 绘制一个房子形状，5个顶点构成
    fColor = gs_in[0].color;
    vec4 position = gl_in[0].gl_Position;

    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:左下  
    EmitVertex();   
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:右下
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:左上
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:右上
    EmitVertex();

    fColor = vec3(1.0, 1.0, 1.0); // 顶部颜色白色 ,随便改
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:顶部
    EmitVertex();

    EndPrimitive();  

}