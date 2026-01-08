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

    // 查询OpenGL限制
    GLint comp;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &comp);
    printf("硬件允许的最大 float 分量数 = %d\n", comp);
    printf("硬件允许的uniform 的KB数 = %d\n", comp * 4 / 1024);
    
    GLint vectors;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &vectors);
    printf("硬件允许的最大 vec4 分量数 (vec4) = %d\n", vectors);
    
    GLint blocks;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &blocks);
    printf("允许绑定到同一个顶点着色器的 UBO 数量上限 = %d\n", blocks);

    // 清理
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

