#version 430 core
layout (points) in; // 声明从顶点着色器接收的图元类型为points
layout (line_strip, max_vertices = 2) out; // 声明输出图元类型为line_strip，最大顶点数为2.(points line_strip triangle_strip), 超过了这个值，OpenGL将不会绘制多出的顶点

// 使用内置的gl_PerVertex结构体来访问顶点数据，不需要定义，直接用gl_in访问即可
// in gl_PerVertex{ 
//     vec4 gl_Position;
//     float gl_PointSize;
//     float gl_ClipDistance[];
// } gl_in[]; // 数组，包含一个图元的所有顶点

void main(){
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); // 第一个顶点位置
    EmitVertex(); // 发射第一个顶点,添加到输出图元中

    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0); 
    EmitVertex(); 

    EndPrimitive(); // 结束图元的构建,所有发射出的顶点被合成为指定的输出图元
}