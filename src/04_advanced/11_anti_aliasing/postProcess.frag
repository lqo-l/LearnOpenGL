#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int effect; // 后期处理效果选择

// 后期处理效果
#define NONE            0 // 无效果
#define INVERT          1 // 反色效果
#define GRAYSCALE       2 // 灰度效果
#define SEPIA           3 // 棕褐色效果
#define BLUR            4 // 高斯模糊效果
#define SHARPEN         5 // 锐化效果
#define EDGE_DETECTION  6 // 边缘检测效果
#define Sobel_X         7 // 索贝尔边缘检测X方向
#define Sobel_Y         8 // 索贝尔边缘检测Y方向
#define Laplacian       9 // 拉普拉斯边缘检测
#define Emboss         10 // 浮雕效果
#define Horizontal_BLUR   11 // 横向拖影效果
#define Vertical_BLUR     12 // 纵向拖影效果
#define Cross           13 // 十字效果

/**
    @brief 卷积函数
    @param tex 采样纹理
    @param uv 纹理坐标
    @param kernel 卷积核
    @param offsetSize 纹理偏移量
    @return 卷积结果颜色,vec3
*/
vec3 conv(sampler2D tex, vec2 uv, float kernel[9], float offsetSize){
    vec3 result = vec3(0.0);
    vec2 offsets[9] = vec2[](
        vec2(-1,  1), vec2( 0,  1), vec2( 1,  1),
        vec2(-1,  0), vec2( 0,  0), vec2( 1,  0),
        vec2(-1, -1), vec2( 0, -1), vec2( 1, -1)
    );
    for(int i = 0; i<9; i++){
        result += kernel[i] * texture(tex, uv + offsetSize * offsets[i]).xyz;
    }
    return result;
}


void main()
{   
    switch(effect){
    case NONE:
        FragColor = texture(screenTexture, TexCoords);
        break;
    case INVERT:{
        FragColor = vec4(1-texture(screenTexture, TexCoords).rgb, 1.0);
        break;
    }
    case GRAYSCALE:{
        vec4 color = texture(screenTexture, TexCoords);
        float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        FragColor = vec4(vec3(gray), 1.0);
        break;
    }
    case SEPIA:{
        vec4 color = texture(screenTexture, TexCoords);
        // 棕褐色调色彩变换矩阵
        vec3 sepia;
        sepia.r = dot(color.rgb, vec3(0.393, 0.769, 0.189));
        sepia.g = dot(color.rgb, vec3(0.349, 0.686, 0.168));
        sepia.b = dot(color.rgb, vec3(0.272, 0.534, 0.131));
        FragColor = vec4(sepia, 1.0);
        break;
    }

    case BLUR:{ // 高斯模糊卷积核
        float kernel[9] = float[](
            1.0/16, 2.0/16, 1.0/16,
            2.0/16, 4.0/16, 2.0/16,
            1.0/16, 2.0/16, 1.0/16 
        );
        float offset = 1.0 / 300.0; // 增大偏移量使效果更明显
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }

    case SHARPEN:{
        float kernel[9] = float[](
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }

    case EDGE_DETECTION:{
        float kernel[9] = float[](
            1,  1, 1,
            1, -8, 1,
            1,  1, 1 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }
    case Sobel_X:{
        float kernel[9] = float[](
            -1, 0, 1,
            -2, 0, 2,
            -1, 0, 1 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }
    case Sobel_Y:{
        float kernel[9] = float[](
            -1, -2, -1,
             0,  0,  0,
             1,  2,  1 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }
    case Laplacian:{
        float kernel[9] = float[](
             0,  1, 0,
             1, -4, 1,
             0,  1, 0 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }
    case Emboss:{
        float kernel[9] = float[](
            -2, -1, 0,
            -1,  1, 1,
             0,  1, 2 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset) + 0.5, 1.0);
        break;
    }
    case Horizontal_BLUR:{
        float kernel[9] = float[](
            1.0,    1.0,    1.0,
            0.0,    1.0,    0.0,
            -1.0,  -1.0,   -1.0 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }
    case Vertical_BLUR:{
        float kernel[9] = float[](
            1.0,    0.0,   -1.0,
            1.0,    1.0,   -1.0,
            1.0,    0.0,   -1.0 
        );
        float offset = 1.0 / 300.0;
        FragColor = vec4(conv(screenTexture, TexCoords, kernel, offset), 1.0);
        break;
    }

    default:
        FragColor = texture(screenTexture, TexCoords);
        break;
    }
}