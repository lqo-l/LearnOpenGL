#pragma once

#include <glad/glad.h>

#include <string>
#include <format>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Shader
{
public:
	unsigned int ID;
	// 读取构建着色器（geometryPath 可选）
	Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr);

	Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometryPath = "")
		: Shader(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.empty() ? nullptr : geometryPath.c_str()){}
	
	~Shader()
	{
		glDeleteProgram(ID);
	}
	// 激活程序
	void use();
	// uniform工具函数
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setMat4(const std::string &name, glm::mat4 value) const;
	void setVec4(const std::string& name, glm::vec4 value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec3(const std::string& name, float r, float g, float b) const;
	void setVec2(const std::string& name, glm::vec2 value) const;
	void setVec2(const std::string& name, float r, float g) const;

private:
	// 辅助函数：读取 shader 文件内容
	static std::string loadShaderSource(const char *filePath);
	// 辅助函数：编译单个 shader
	static unsigned int compileShader(GLenum shaderType, const char *source, const char *filePath);
	// 辅助函数：检查编译/链接错误
	static void checkCompileErrors(unsigned int shader, const std::string &type, const char *filePath = nullptr);
};

// ==================== 辅助函数实现 ====================

inline std::string Shader::loadShaderSource(const char *filePath)
{
	std::ifstream shaderFile;
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		shaderFile.open(filePath);
		std::stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		shaderFile.close();
		return shaderStream.str();
	}
	catch (const std::ifstream::failure &e)
	{
		std::cerr << std::format("[ERROR]: Shader file not read: {}\n", filePath);
		throw std::runtime_error(std::string("Cannot open shader file: ") + filePath);
	}
}

inline void Shader::checkCompileErrors(unsigned int shader, const std::string &type, const char *filePath)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM") // 单个 shader 的编译错误
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << std::format("[ERROR]: Shader::{}::Compilation_failed\nFile: {}\n{}",
									 type, filePath ? filePath : "unknown", infoLog) << std::endl;
		}
	}
	else // 整个 shader 程序的链接错误
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << std::format("[ERROR]: Shader::PROGRAM::Linking_failed\n{}", infoLog) << std::endl;
		}
	}
}

inline unsigned int Shader::compileShader(GLenum shaderType, const char *source, const char *filePath)
{
	unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	
	// 获取类型名称用于错误信息
	std::string typeName;
	switch (shaderType)
	{
		case GL_VERTEX_SHADER:   typeName = "VERTEX"; break;
		case GL_FRAGMENT_SHADER: typeName = "FRAGMENT"; break;
		case GL_GEOMETRY_SHADER: typeName = "GEOMETRY"; break;
		default: typeName = "UNKNOWN"; break;
	}
	checkCompileErrors(shader, typeName, filePath);
	return shader;
}

// ==================== 构造函数实现 ====================

inline Shader::Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath)
{
	// 1. 读取 shader 源码
	std::string vertexCode = loadShaderSource(vertexPath);
	std::string fragmentCode = loadShaderSource(fragmentPath);
	std::string geometryCode;
	if (geometryPath != nullptr)
	{
		geometryCode = loadShaderSource(geometryPath);
	}

	// 2. 编译 shaders
	unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), vertexPath);
	unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), fragmentPath);
	unsigned int geometry = 0;
	if (geometryPath != nullptr)
	{
		geometry = compileShader(GL_GEOMETRY_SHADER, geometryCode.c_str(), geometryPath);
	}

	// 3. 链接 shader 程序
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != nullptr)
	{
		glAttachShader(ID, geometry);
	}
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	// 4. 删除已链接的 shader 对象
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
	{
		glDeleteShader(geometry);
	}
}

inline void Shader::use()
{
	glUseProgram(ID);
}
// uniform工具函数
inline void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
inline void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
inline void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
inline void Shader::setMat4(const std::string &name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
inline void Shader::setVec4(const std::string& name, glm::vec4 value) const {
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); // &value[0]写法获取指针也可以
}
inline void Shader::setVec3(const std::string& name, glm::vec3 value) const {
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
inline void Shader::setVec3(const std::string& name, float r, float g, float b) const{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(glm::vec3(r, g, b)));
}
inline void Shader::setVec2(const std::string& name, glm::vec2 value) const {
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}
inline void Shader::setVec2(const std::string& name, float r, float g) const{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(glm::vec2(r, g)));
}
