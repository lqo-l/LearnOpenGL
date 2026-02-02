#version 430 core
// 这个着色器手动计算深度

in vec4 fragWorldPos;

uniform vec3 lightPos; // 光源位置
uniform float far_plane; // 远裁剪面距离

void main()
{	
    float Distance = length(fragWorldPos.xyz - lightPos);

    Distance = Distance / far_plane;

    gl_FragDepth = Distance; 
}