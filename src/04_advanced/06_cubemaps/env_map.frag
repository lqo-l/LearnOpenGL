#version 430 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragWorldPos;

out vec4 FragColor;

uniform sampler2D texture1;
uniform samplerCube skybox;
uniform vec3 cameraPos;
uniform int effect; // 0: no effect, 1: 反射, 2: 折射

void main()
{    
    switch(effect){
        case 0:{
            FragColor = texture(texture1, TexCoords);
            break;
        }
        case 1:{
            vec3 I = normalize(FragWorldPos - cameraPos);
            vec3 R = reflect(I, normalize(Normal));
            FragColor = texture(skybox, R); 
            break;
        }
        case 2:{ // 仅单色折射效果
            vec3 I = normalize(FragWorldPos - cameraPos);
            float ratio = 1.00 / 1.52; // 空气折射率/玻璃折射率，假设箱子是玻璃材质的
            vec3 R = refract(I, normalize(Normal), ratio);
            FragColor = vec4(texture(skybox, R).rgb, 1.0);
            break;
        }
    }    
}