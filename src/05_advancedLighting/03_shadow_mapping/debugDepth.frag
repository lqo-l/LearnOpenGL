#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;

uniform float near;
uniform float far;
uniform bool needToLinearizeDepth;

// 透视投影后，深度非线性，可以通过该函数转回线性深度
float LinearizeDepth(float depth){
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));    
}
void main()
{    
    float depthValue = texture(depthMap, TexCoords).r; // texture返回vec4，我们只需要其中的r分量
    float depth;
    if(needToLinearizeDepth){
         depth = (LinearizeDepth(gl_FragCoord.z) - near) / (far - near) ; // 透视投影用, 这里线性化后进行归一化处理 [near,far]->[0,1]
    }else{
         depth = depthValue; // 正交投影不需要线性化，本身也在[0,1]不需要归一化
    }

    FragColor = vec4(vec3(depth), 1.0);
}