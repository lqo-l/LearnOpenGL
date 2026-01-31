#version 430 core

in vec2 TexCoords;
in vec3 normal;
in vec3 fragWorldPos;

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

uniform Material material;

uniform vec3 viewPos;
// uniform sampler2D texture1; // removed unused texture

uniform DirLight parallel;
uniform PointLight point;
uniform SpotLight spot;

uniform bool openParallelLight;
uniform bool openPointLight;
uniform bool openSpotLight;

vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 texKd, vec3 texKs);
vec3 calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs);
vec3 calcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs);

uniform bool useBlinnPhong;
uniform bool gammaCorrection; // 是否在shader中手动进行gamma校正
uniform bool manualTex2Linear; // 是否手动将采样的纹理从sRGB空间转换为线性空间

void main()
{	

    vec3 norm = normalize(normal); // world space下的法线
    vec3 viewDir = normalize(viewPos - fragWorldPos); 
    vec3 texKd = texture(material.texture_diffuse, TexCoords).rgb;
    vec3 texKs = vec3(material.texture_specular); 

    if(manualTex2Linear){
        // Kd是在sRGB空间下采样得到的漫反射贴图颜色，需要先转换为线性空间进行光照计算
        texKd = pow(texKd, vec3(2.2));
    }

    // 多光源累加
    vec3 result = vec3(0);
    if(openParallelLight){
        result += calcDirLight(parallel, norm, viewDir, texKd, texKs);
    }
    if(openPointLight){
        result += calcPointLight(point, norm, fragWorldPos, viewDir, texKd, texKs);
    }

    vec3 spotColor = calcSpotLight(spot, norm, fragWorldPos, viewDir, texKd, texKs);
    if(openSpotLight){
        result += spotColor;
    }

    // gamma校正
    if(gammaCorrection){
        float gamma = 2.2;
        result.rgb = pow(result.rgb, vec3(1.0/gamma)); // 恢复sRGB空间
    }
  
    
    // 总颜色
    FragColor = vec4(result, 1.0);
}


// 坐标和向量均为世界坐标下
vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 texKd, vec3 texKs){
    vec3 result = vec3(0);
    norm = normalize(norm);
    viewDir = normalize(viewDir);

    result += light.Ia * texKd * light.lightColor;
    vec3 lightDir = normalize(-light.direction);
    result += light.Id * texKd * light.lightColor * max(0,dot(lightDir,norm));

    if(useBlinnPhong){
        vec3 h = normalize(lightDir + viewDir); // 半程向量
        result += light.Is * texKs * light.lightColor * pow(max(dot(norm, h),0.0),material.shininess) ;
        return result;
    }else{
        vec3 reflectDir = reflect(-lightDir, norm);
        result += light.Is * texKs * light.lightColor * pow(max(dot(viewDir, reflectDir),0.0),material.shininess) ;
    }

    return result;
}

// 坐标和向量均为世界坐标下
vec3 calcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs){
    vec3 result = vec3(0);
    norm = normalize(norm);
    viewDir = normalize(viewDir);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    result += light.Ia * texKd * light.lightColor * attenuation;
    vec3 lightDir = normalize(light.position - fragPos);
    result += light.Id * texKd * light.lightColor * max(0,dot(norm, lightDir)) * attenuation;

    if(useBlinnPhong){
        vec3 h = normalize(lightDir + viewDir); // 半程向量
        result += light.Is * texKs * light.lightColor * pow(max(dot(norm, h),0.0), material.shininess) * attenuation;
    }else{
        vec3 reflectDir = reflect(-lightDir, norm);
        result += light.Is * texKs * light.lightColor * pow(max(dot(viewDir, reflectDir),0.0), material.shininess) * attenuation;
    }

    return result;
}

// 坐标和向量均为世界坐标下
vec3 calcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texKd, vec3 texKs){
    vec3 result = vec3(0);
    norm = normalize(norm);
    viewDir = normalize(viewDir);

    float distance = length(spot.position - fragPos);
    vec3 lightDir = normalize(spot.position - fragPos); // frag2lightPos
    float attenuation = 1.0 / (spot.constant + spot.linear * distance + spot.quadratic * (distance*distance));
    float epsilon = spot.cutOff - spot.outerCutOff; // 分母
    float lightRadian = dot(lightDir, -spot.spotDir); // 计算光线与聚光灯中心方向的余弦值(注意：spot.spotDir是光源指向物体，需要取反与lightDir保持一致)
    float intensity = (lightRadian - spot.outerCutOff)/ epsilon; // 线性插值计算强度，角度越大强度越小。角度大于等于外光锥时，强度为0，角度小于内光锥时，强度为1
    intensity = clamp(intensity, 0.f, 1.f); // 限制在0-1之间

    result += spot.Ia * texKd * spot.lightColor * attenuation; // 环境光不受聚光强度影响
    result += spot.Id * texKd * spot.lightColor * max(0, dot(norm, lightDir)) * attenuation * intensity;

    if(useBlinnPhong){
        vec3 h = normalize(lightDir + viewDir); // 半程向量
        result += spot.Is * spot.lightColor * texKs * pow(max(0, dot(h, norm)), material.shininess) * attenuation * intensity;
    }else{
        vec3 reflectDir = reflect(-lightDir, norm);
        result += spot.Is * spot.lightColor * texKs * pow(max(0, dot(viewDir, reflectDir)), material.shininess) * attenuation * intensity;
    }

    return result;
}
