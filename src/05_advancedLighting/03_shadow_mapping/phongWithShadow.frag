#version 430 core


in VS_OUT{
    vec3 normal;
    vec3 fragWorldPos;
    vec2 TexCoords;
    vec4 fragPosLightSpace; // 片元在光源空间的坐标(裁剪空间坐标，投影后的坐标)
}fs_in;

out vec4 FragColor;


struct Material {
    sampler2D texture_diffuse;
    float texture_specular;
    float shininess; // 高光指数
};

struct DirLight{ // 定向光（平行光），无衰减
    vec3 direction; // 平行光方向, 从光源到物体，使用时需取反
    vec3 lightColor; // 光源颜色

    vec3 Ia; // ambient强度(vec3形式，可以对rgb分量分别调整)
    vec3 Id; // diffusion强度
    vec3 Is; // specular强度
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

struct SpotLight{
    vec3 position;
    vec3 lightColor; // 光源颜色

    vec3 spotDir; // 聚光灯的方向
    float cutOff; // 内切光束余弦值
    float outerCutOff; // 外切光束余弦值
    
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
uniform DirLight parallel;
uniform PointLight point;
uniform SpotLight spot;

uniform bool openParallelLight;
uniform bool openPointLight;
uniform bool openSpotLight;

uniform vec3 viewPos;
uniform sampler2D shadowMap; // 阴影贴图

// 函数声明
LightingResult calcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 texKd, vec3 texKs);
LightingResult calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs);
LightingResult calcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs);
LightingResult addLighting(LightingResult a, LightingResult b); // 光照结果累加, 工具函数

// 需要计算frag在光源空间的坐标，与阴影贴图比较大小
float ShadowCalculation(vec4 fragPosLightSpace){
    // 透视除法，转换到归一化设备坐标NDC空间
    fragPosLightSpace.xyz /= fragPosLightSpace.w; // 如果是正交投影，则w=1，不影响

    // 转换到[0,1]区间（屏幕空间），作为纹理坐标使用
    vec3 projCoords = fragPosLightSpace.xyz * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; // 纹理中存储的深度值
    float currentDepth = projCoords.z; // 片元在光源空间的深度值
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0; // 简单比较，当前深度大于纹理深度则在阴影中

    return shadow;
}

void main()
{	
    vec3 norm = normalize(fs_in.normal); // world space下的法线
    vec3 viewDir = normalize(viewPos - fs_in.fragWorldPos); 
    vec3 texKd = texture(material.texture_diffuse, fs_in.TexCoords).rgb;
    vec3 texKs = vec3(material.texture_specular); 

    // 多光源累加
    LightingResult result = LightingResult(vec3(0), vec3(0), vec3(0));

    if(openParallelLight){
        result = addLighting(result, calcDirLight(parallel, norm, viewDir, texKd, texKs));
    }
    if(openPointLight){
        result = addLighting(result, calcPointLight(point, norm, fs_in.fragWorldPos, viewDir, texKd, texKs));
    }
    if(openSpotLight){
        result = addLighting(result, calcSpotLight(spot, norm, fs_in.fragWorldPos, viewDir, texKd, texKs));
    }

    // 阴影
    float shadow = ShadowCalculation(fs_in.fragPosLightSpace); // 计算阴影
    vec3 color = vec3(0.0);
    color = (result.ambient) + (1.0 - shadow) * (result.diffuse + result.specular); // 让阴影只影响漫反射和镜面反射部分，保留环境光

    // 总颜色
    FragColor = vec4(color, 1.0);
}


// 坐标和向量均为世界坐标下
LightingResult calcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 texKd, vec3 texKs){
    LightingResult result;
    norm = normalize(norm);
    viewDir = normalize(viewDir);

    result.ambient = light.Ia * texKd * light.lightColor;
    vec3 lightDir = normalize(-light.direction);
    result.diffuse = light.Id * texKd * light.lightColor * max(0,dot(lightDir,norm));

    vec3 h = normalize(lightDir + viewDir); // 半程向量
    result.specular = light.Is * texKs * light.lightColor * pow(max(dot(norm, h),0.0),material.shininess) ;
    return result;
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

// 坐标和向量均为世界坐标下
LightingResult calcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs){
    LightingResult result;

    norm = normalize(norm);
    viewDir = normalize(viewDir);

    float distance = length(light.position - fragPos);
    vec3 lightDir = normalize(light.position - fragPos); // frag2lightPos
    float attenuation = 1.0 / (light.constant + light.linear * distance + spot.quadratic * (distance*distance));
    float epsilon = light.cutOff - light.outerCutOff; // 分母
    float lightRadian = dot(lightDir, -light.spotDir); // 计算光线与聚光灯中心方向的余弦值(注意：spot.spotDir是光源指向物体，需要取反与lightDir保持一致)
    float intensity = (lightRadian - light.outerCutOff)/ epsilon; // 线性插值计算强度，角度越大强度越小。角度大于等于外光锥时，强度为0，角度小于内光锥时，强度为1
    intensity = clamp(intensity, 0.0, 1.0); // 限制在0-1之间

    result.ambient = light.Ia * texKd * light.lightColor * attenuation; // 环境光不受聚光强度影响
    result.diffuse = light.Id * texKd * light.lightColor * max(0, dot(norm, lightDir)) * attenuation * intensity;

    vec3 h = normalize(lightDir + viewDir); // 半程向量
    result.specular = light.Is * light.lightColor * texKs * pow(max(0, dot(h, norm)), material.shininess) * attenuation * intensity;

    return result;
}

LightingResult addLighting(LightingResult a, LightingResult b){
    LightingResult result;
    result.ambient = a.ambient + b.ambient;
    result.diffuse = a.diffuse + b.diffuse;
    result.specular = a.specular + b.specular;
    return result;
}