#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>

int main(int argc, char const *argv[])
{
    std::cout << "C++ Version: " << __cplusplus << std::endl;

    // 初始化GLFW
    if (!glfwInit()) {
        std::cout << "GLFW初始化失败" << std::endl;
        return -1;
    }

    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口（必须创建窗口才能有OpenGL上下文）
    GLFWwindow* window = glfwCreateWindow(800, 600, "GL Uniform Size Test", NULL, NULL);
    if (!window) {
        std::cout << "创建GLFW窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 加载所有OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "GLAD初始化失败" << std::endl;
        glfwTerminate();
        return -1;
    }

        // 顶点着色器可用的标量uniform数，4096
    GLint comp;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &comp);
    printf("Nums of uniform scalar that a vertex shader can declare: %d\n", comp); // 顶点着色器可使用的非block uniform 标量组件的最大数量,编译期常量额度,存储于寄存器

    // 单个uniform block的最大尺寸 64KB
	GLint maxUniformBlockSize;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
	std::cout<< "max size of a uniform block(KB) : "<< maxUniformBlockSize/1024 << std::endl; 

    // 最多可绑定的uniform binding points绑定槽位数  84
    GLint maxUniformBufferBindings;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
    std::cout<< "max num of uniform buffer bindings: " << maxUniformBufferBindings << std::endl;

    // 所有着色器阶段可用的uniform blocks数量  84
    GLint maxCombinedUniformBlocks;
    glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &maxCombinedUniformBlocks);
    std::cout<<"max num of combined uniform blocks: " << maxCombinedUniformBlocks << std::endl;

    GLint uniformBufferOffsetAlign = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniformBufferOffsetAlign);
    std::cout<< "uniform buffer offset alignment: " << uniformBufferOffsetAlign << std::endl; // glBindBufferRange的offset必须是该值的整数倍

    // 清理
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

