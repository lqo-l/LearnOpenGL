#version 430 core
layout (triangles) in; 
layout (triangle_strip, max_vertices = 3) out; 


in VS_OUT{
    vec2 texCoord;
    vec3 normal;
}gs_in[];

uniform float explodeFactor;
uniform mat4 projection;

out vec2 texCoord;

void main(){
    // 传入的坐标和法线是视图空间的，沿法线方向移动顶点

    for(int i = 0; i<3; i++){
        vec3 normal = normalize(gs_in[i].normal);
        vec4 position = gl_in[i].gl_Position + vec4(normal * explodeFactor, 0.0);
        gl_Position = projection * position;
        texCoord = gs_in[i].texCoord;  // 传递纹理坐标
        EmitVertex();
    }

    EndPrimitive();
}