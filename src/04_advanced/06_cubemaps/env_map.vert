#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragWorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// 计算世界空间位置和法线，因为需要计算世界空间的反射向量以采样skybox的"世界纹理坐标(世界坐标下的三维方向向量)"
void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragWorldPos = (model * vec4(aPos, 1.0)).xyz;
}