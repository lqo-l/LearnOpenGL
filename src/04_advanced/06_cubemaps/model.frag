#version 430 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragWorldPos;

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
uniform float Ia; // 环境光强度

uniform samplerCube skybox;
uniform vec3 cameraPos;
uniform int effect; // 0: no effect, 1: 反射, 2: 折射

void main(){
    vec3 result = vec3(0.0);

    // diffuse贴图当作模型颜色，设置一定环境光
    result += Ia * texture(material.texture_diffuse1, TexCoords).rgb;

    vec3 N = normalize(Normal);
    // 未做镜面反射、漫反射；未使用法线贴图。

    //  反射或折射
    switch(effect){
        case 0:{
            break;
        }
        case 1:{
            vec3 I = normalize(FragWorldPos - cameraPos);
            vec3 R = reflect(I, N);
            result += texture(skybox, R).rgb * texture(material.texture_ambient1, TexCoords).rgb; 
            break;
        }
        case 2:{ // 仅单色折射效果
            vec3 I = normalize(FragWorldPos - cameraPos);
            float ratio = 1.00 / 1.52; // 空气折射率/玻璃折射率，假设箱子是玻璃材质的
            vec3 R = refract(I, N, ratio);
            result += texture(skybox, R).rgb * texture(material.texture_ambient1, TexCoords).rgb;
            break;
        }
    }    

    FragColor = vec4(result, 1.0);
}