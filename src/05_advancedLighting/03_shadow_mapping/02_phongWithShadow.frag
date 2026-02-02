#version 430 core


in VS_OUT{
    vec3 normal;
    vec3 fragWorldPos;
    vec2 TexCoords;
    // vec4 fragPosLightSpace; // 片元在光源空间的坐标(裁剪空间坐标，投影后的坐标)
}fs_in;

out vec4 FragColor;


struct Material {
    sampler2D texture_diffuse;
    float texture_specular;
    float shininess; // 高光指数
};


struct PointLight{
    vec3 position;
    vec3 lightColor; // 光源颜色

    // 衰减系数
    float constant; // 一般为1，确保分母不小于1
    float linear;   // 主导小距离衰减
    float quadratic; // 主导大距离衰减

    vec3 Ia; // ambient强度(vec3形式，可以对rgb分量分别调整)
    vec3 Id; // diffusion强度
    vec3 Is; // specular强度
};

struct LightingResult{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform PointLight point;

uniform bool openPointLight;

uniform vec3 viewPos;
uniform samplerCube depthCubeMap; // 深度立方体贴图（点光源阴影）
uniform float far_plane; // 远裁剪面距离

uniform bool pcfEnabled; // 是否开启PCF
uniform bool adaptiveShadowBias; // 是否开启自适应阴影偏移
uniform bool OptimizedPcf; // 优化pcf，从垂直方向采样
uniform bool debugShadowMap; // 调试阴影贴图

vec3 sampleOffsetDirections[20] = vec3[] // 20个不同的采样偏移方向，立方体8个顶点方向+12个边中心方向
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

// 函数声明
LightingResult calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs);
LightingResult addLighting(LightingResult a, LightingResult b); // 光照结果累加, 工具函数

// 需要计算frag在光源空间的坐标，与阴影贴图比较大小
float ShadowCalculation(vec3 fragWorldPos){
    vec3 fragToLight = fragWorldPos - point.position;   
    float currentDepth = length(fragToLight); // 片元在光源空间的深度值

    float bias = 0.005;  // 阴影偏移，防止阴影失真(自阴影面板条纹现象)
    if(adaptiveShadowBias){
        vec3 normal = normalize(fs_in.normal);
        vec3 lightDir = normalize(point.position - fragWorldPos);
        bias = max(0.05 * (1-dot(normal, lightDir)) , 0.005); // 根据法线和光线方向计算偏移量(斜面偏移更大)
    }

    float shadow = 0.0;
    if(pcfEnabled && !debugShadowMap){ 
        if(OptimizedPcf){
            // 在20个相对独立的方向采样
            shadow = 0.0; 
            float bias = 0.15;
            int samples = 20;
            float viewDistance = length(viewPos - fragWorldPos);
            // float diskRadius = 0.05; // 偏移
            float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0; // 根据视距动态调整偏移半径，越远偏移越大，阴影更柔和
            for(int i = 0; i < samples; ++i){
                float closestDepth = texture(depthCubeMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
                closestDepth *= far_plane;   // Undo mapping [0;1]
                shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
            }
            shadow /= float(samples);
        }else{
            // 在投影方向三个维度采样多个点，计算阴影
            float bias = 0.05; 
            float samples = 4.0; // 64次采样
            float offset = 0.1;
            for(float x = -offset; x < offset; x += 2*offset / samples)
            {
                for(float y = -offset; y < offset; y += 2*offset / samples)
                {
                    for(float z = -offset; z < offset; z += 2*offset / samples)
                    {
                        float closestDepth = texture(depthCubeMap, fragToLight + vec3(x, y, z)).r; 
                        closestDepth *= far_plane;   // 原始深度值
                        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
                    }
                }
            }
            shadow /= (samples * samples * samples);
        }
    }
    else{
        float closestDepth = texture(depthCubeMap, fragToLight).r; // 方向向量不需要是单位向量，无需归一化。
        if(debugShadowMap){
            return closestDepth; // 返回归一化深度值进行调试
        }
        closestDepth *= far_plane; // 恢复原始深度值
        shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0; // 让当前视角的深度更近一些
    }
   
    return shadow;
}

void main()
{	
    if(debugShadowMap){
        float depth = ShadowCalculation(fs_in.fragWorldPos); // 计算阴影
        FragColor = vec4(vec3(depth), 1.0);
        return;
    }

    vec3 norm = normalize(fs_in.normal); // world space下的法线
    vec3 viewDir = normalize(viewPos - fs_in.fragWorldPos); 
    vec3 texKd = texture(material.texture_diffuse, fs_in.TexCoords).rgb;
    vec3 texKs = vec3(material.texture_specular); 

    // 多光源累加
    LightingResult result = LightingResult(vec3(0), vec3(0), vec3(0));

    if(openPointLight){
        result = addLighting(result, calcPointLight(point, norm, fs_in.fragWorldPos, viewDir, texKd, texKs));
    }

    // 阴影
    float shadow = ShadowCalculation(fs_in.fragWorldPos); // 计算阴影
    vec3 color = vec3(0.0);
    color = (result.ambient) + (1.0 - shadow) * (result.diffuse + result.specular); // 让阴影只影响漫反射和镜面反射部分，保留环境光

    // 总颜色
    FragColor = vec4(color, 1.0);
}


// 坐标和向量均为世界坐标下
LightingResult calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs){
    LightingResult result;
    norm = normalize(norm);
    viewDir = normalize(viewDir);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    result.ambient = light.Ia * texKd * light.lightColor * attenuation;
    vec3 lightDir = normalize(light.position - fragPos);
    result.diffuse = light.Id * texKd * light.lightColor * max(0,dot(norm, lightDir)) * attenuation;

    vec3 h = normalize(lightDir + viewDir); // 半程向量
    result.specular = light.Is * texKs * light.lightColor * pow(max(dot(norm, h),0.0), material.shininess) * attenuation;

    return result;
}

LightingResult addLighting(LightingResult a, LightingResult b){
    LightingResult result;
    result.ambient = a.ambient + b.ambient;
    result.diffuse = a.diffuse + b.diffuse;
    result.specular = a.specular + b.specular;
    return result;
}