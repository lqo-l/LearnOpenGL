#version 430 core

in vec2 TexCoords;
// in vec3 Normal;
// in vec3 FragWorldPos;

out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;     // 漫反射贴图
    sampler2D texture_diffuse2; 
    sampler2D texture_diffuse3;
    sampler2D texture_diffuse4;
    sampler2D texture_specular1;    // 镜面反射贴图
    sampler2D texture_specular2;
    sampler2D texture_normal1;      // 切线空间法线贴图
    sampler2D texture_ambient1; // 这里用作反射环境贴图
};
uniform Material material;
// uniform float Ia; // 环境光强度

uniform vec3 cameraPos;

void main(){
    vec3 result = vec3(0.0);

    // diffuse贴图当作模型颜色，设置一定环境光
    result += 1.0 * texture(material.texture_diffuse1, TexCoords).rgb;

    FragColor = vec4(result, 1.0);
}