#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out VS_OUT{
    vec3 normal;
    vec3 fragWorldPos;
    vec2 TexCoords;
    // vec4 fragPosLightSpace; // 片元在光源空间的坐标(裁剪空间坐标，投影后的坐标) // 不再需要，现在直接用fragWorldPos计算距离
}vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
// uniform mat4 lightSpaceMatrix;

uniform bool reverse_normals;

void main(){
    if(reverse_normals){ // 渲染盒子内部时反转法线的hack（本来内部模型应该法线朝内，这里用法线朝外的顶点就hack一下，在shader里反转法线）
        vs_out.normal = -mat3(transpose(inverse(model))) * aNormal;
    }else{
        vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    }
    vs_out.fragWorldPos = vec3(model * vec4(aPos, 1.0)); // 片元的世界坐标，用于光照计算
    vs_out.TexCoords = aTexCoord;
    // vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragWorldPos, 1.0); // 片元在光源空间的坐标
    gl_Position = projection * view * model * vec4(aPos, 1.0);

}