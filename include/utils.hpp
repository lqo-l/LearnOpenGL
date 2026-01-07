#pragma once

#include <glad/glad.h>

#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

// 简单的最近邻缩放函数
inline unsigned char* resizeImage(unsigned char* input, int inWidth, int inHeight, int outWidth, int outHeight, int channels) {
    unsigned char* output = new unsigned char[outWidth * outHeight * channels];
    
    float xRatio = (float)inWidth / outWidth;
    float yRatio = (float)inHeight / outHeight;
    
    for(int y = 0; y < outHeight; y++) {
        for(int x = 0; x < outWidth; x++) {
            int srcX = (int)(x * xRatio);
            int srcY = (int)(y * yRatio);
            
            for(int c = 0; c < channels; c++) {
                output[(y * outWidth + x) * channels + c] = 
                    input[(srcY * inWidth + srcX) * channels + c];
            }
        }
    }
    
    return output;
}

#include <assimp/scene.h>

namespace fs = std::filesystem;

/**
 * @brief Get the Absolute Dir string of filePath
 * 
 * @param filePath File path(relative or absolute)
 * @return std::string. Directory path string
 */
inline std::string getDir(const char *filePath)
{
	return fs::absolute(fs::path(filePath)).parent_path().string();
}

/**
 * @brief 获取相对于可执行文件的素材文件绝对路径
 * 
 * @param argv0 可执行文件路径
 * @param relativePath 素材文件相对于可执行文件所在目录的相对路径
 * @return std::string 素材绝对路径
 */
inline std::string getAssetAbsPath(const char *argv0, const std::string &relativePath)
{
	// 把相对路径和可执行文件目录合并后再做归一化。
	// 之前的实现是在右侧的相对路径上调用 lexically_normal(),
	// 导致合并后仍可能包含 ".." 等成分而没有被消除。
	fs::path base = fs::path(getDir(argv0));
	fs::path combined = base / fs::path(relativePath);
	// 先转为绝对路径再做 lexically_normal()，这样能正确清理多级相对路径成分。
	return fs::absolute(combined).lexically_normal().string();
}

/// <summary>
/// 根据模型的位置和lookat构建模型的model矩阵（从模型空间到世界空间），
///注：view构建用glm::lookAt,这里用于构造model的。区别在于：相机的front是-z；此外view矩阵是world->camera(model)，model矩阵是model->world。
/// </summary>
/// <param name="pos">模型位置</param>
/// <param name="target">看向的目标</param>
/// <param name="up">上向量</param>
/// <returns>model矩阵</returns>
inline glm::mat4 ModelLookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f))
{
	glm::mat4 model(1.f);

	glm::vec3 front(normalize(target - pos));			// z
	glm::vec3 right = normalize(glm::cross(up, front)); // x
	up = glm::cross(front, right);						// y

	model[0] = glm::vec4(right, 0);
	model[1] = glm::vec4(up, 0);
	model[2] = glm::vec4(front, 0);
	model[3] = glm::vec4(pos, 1);

	return model; // 模型->世界的变换矩阵model
}

/**
 * @brief 加载纹理
 * 
 * @param path 纹理绝对路径或相对于可执行文件的路径
 * @param warpS 纹理S轴包装模式，默认GL_REPEAT,可选GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
 * @param warpT 纹理T轴包装模式，默认GL_REPEAT,可选
 * @param minFilter 缩小过滤器，默认GL_LINEAR_MIPMAP_LINEAR,可选GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR, GL_NEAREST等
 * @param magFilter 放大过滤器，默认GL_LINEAR,可选GL_NEAREST
 * @param borderColor 当warp模式为GL_CLAMP_TO_BORDER时，设置边界颜色，默认不设置为纯黑
 * 
 * @return unsigned int 纹理对象ID
 */
inline unsigned int loadTexture(const char *path, GLenum warpS = GL_REPEAT, GLenum warpT = GL_REPEAT, GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR, GLenum magFilter = GL_LINEAR, std::optional<glm::vec4> borderColor = std::nullopt, bool flip = true)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	if(flip){
		stbi_set_flip_vertically_on_load(true); // 加载时反转图片，因为图片原点一般在左上角，opengl屏幕空间原点左下
	}
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warpS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warpT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		if (warpS == GL_CLAMP_TO_BORDER || warpT == GL_CLAMP_TO_BORDER){
			glm::vec4 color = borderColor.value_or(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
		}

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

/**  
 * @brief 从assimp的aiTexture结构体加载纹理
 * 
 * @param aiTex assimp的aiTexture指针
 * @param warpS 纹理S轴包装模式，默认GL_REPEAT,可选GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
 * @param warpT 纹理T轴包装模式，默认GL_REPEAT,可选
 * @param minFilter 缩小过滤器，默认GL_LINEAR_MIPMAP_LINEAR,
 * @param magFilter 放大过滤器，默认GL_LINEAR
 * @param borderColor 当warp模式为GL_CLAMP_TO_BORDER时，设置边界颜色，默认不设置为纯黑
 * 
 * @return unsigned int 纹理对象ID
*/
inline unsigned int loadTextureFromAssimp(const aiTexture *aiTex, GLenum warpS = GL_REPEAT, GLenum warpT = GL_REPEAT, GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR, GLenum magFilter = GL_LINEAR, std::optional<glm::vec4> borderColor = std::nullopt){
	if(aiTex == nullptr){
		return 0;
	}
	unsigned int textureID;
	glGenTextures(1, &textureID);
	
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLenum format = GL_RGBA; // assimp中aiTexture数据格式默认为RGBA8888
	glTexImage2D(GL_TEXTURE_2D, 0, format, aiTex->mWidth, aiTex->mHeight, 0, format, GL_UNSIGNED_BYTE, aiTex->pcData);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warpS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warpT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	if (warpS == GL_CLAMP_TO_BORDER || warpT == GL_CLAMP_TO_BORDER){
		glm::vec4 color = borderColor.value_or(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
	}

	return textureID;
}

// inline unsigned int TextureFromFile(const char *path, const std::string &directory){
//     std::string filename = std::string(path);
//     filename = directory + '/' + filename;
    
//     unsigned int textureID = loadTexture(filename.c_str());
//     return textureID;
// }


/**
 * @brief 加载纹理
 * 
 * @param faces 纹理路径数组：绝对路径或相对于可执行文件的路径
 * 
 * @return unsigned int 纹理对象ID
 */
inline unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;
	int targetSize = -1; // 目标尺寸（取第一个面的尺寸，且强制为正方形）
	
	for(int i = 0; i < faces.size(); i++)
	{
		// stbi_set_flip_vertically_on_load(true); // cubemap一般不需要翻转
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		
		if(!data){
			std::cout << "[ERROR] Texture failed to load at path: " << faces[i] << std::endl;
			continue;
		}
		
		// 第一个面：确定目标尺寸（取较小的边，保证正方形）
		if(i == 0){
			targetSize = std::min(width, height);
			// std::cout << "[INFO] Cubemap target size set to: " << targetSize << "x" << targetSize << std::endl;
		}
		
		unsigned char* finalData = data;
		bool needsFree = false;
		// 尺寸不一致，自动缩放到相同大小
		if(width != targetSize || height != targetSize){
			std::cout << "[INFO] Resizing face " << i << " from " << width << "x" << height 
			          << " to " << targetSize << "x" << targetSize << std::endl;
			
			finalData = resizeImage(data, width, height, targetSize, targetSize, nrComponents);
			needsFree = true;
		}
		
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		// 立方体贴图有6个纹理目标GL_TEXTURE_CUBE_MAP_POSITIVE_X 到 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z，其枚举int值线性增长
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, targetSize, targetSize, 0, format, GL_UNSIGNED_BYTE, finalData);

		// 释放内存
		stbi_image_free(data);
		if(needsFree){
			delete[] finalData;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
	
	return textureID;
}