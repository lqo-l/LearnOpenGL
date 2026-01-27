
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <unordered_map>

#include <shader.hpp>
#include <camera.hpp>
#include <utils.hpp>
#include <model.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

unsigned int SCR_WIDTH = 1440;
unsigned int SCR_HEIGHT = 800;
bool cursorLocked = true;

Camera camera(glm::vec3(0.f, 0.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

// timing
float deltaTime = 0.f, lastTime = 0.f;

double cursorLastX = double(SCR_WIDTH / 2);
double cursorLastY = double(SCR_HEIGHT / 2);
bool firstMouse = true;

int main(int argc, char **argv)
{
    // 初始化
    const char *glsl_version = "#version 430";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Fail to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 加载OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Imgui初始化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // 中文字体
    ImFontConfig config;
    config.MergeMode = false;
    config.OversampleH = 2;
    config.OversampleV = 2;
    const char *fontPath = getAssetAbsPath(argv[0], "C:\\Windows\\Fonts\\msyh.ttc").c_str();                                  // 微软雅黑
    ImFont *font = io.Fonts->AddFontFromFileTTF(fontPath, 18.0f, &config, io.Fonts->GetGlyphRangesChineseSimplifiedCommon()); // GetGlyphRangesChineseFull()
    if (font == nullptr)
    {
        std::cout << "Failed to load font: " << fontPath << std::endl;
        io.Fonts->AddFontDefault(); // 回退到默认字体
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    // 回调注册，设置
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 着色器
    Shader common_shader(getAssetAbsPath(argv[0], "common.vert"), getAssetAbsPath(argv[0], "common.frag"));
    Shader screen_shader(getAssetAbsPath(argv[0], "screen.vert"), getAssetAbsPath(argv[0], "screen.frag")); // 普通四边形屏幕空间渲染
    Shader postProcess_shader(getAssetAbsPath(argv[0], "postProcess.vert"), getAssetAbsPath(argv[0], "postProcess.frag")); // 带后处理的四边形屏幕空间渲染

    float cubeVertices[] = {
        // positions       
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

#pragma region FBO
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // 多采样颜色缓冲纹理
    int samples = 4;
    unsigned int texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texColorBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texColorBuffer, 0); // 把名为 texColorBuffer 的多重采样颜色纹理的第 0 层（mipmap），绑定到当前 FBO 的 0 号颜色输出上

    // 多重采样渲染缓冲对象
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE){
        std::cout << "framebuffer不完整" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 另一个中间fbo，用于resolve多重采样的渲染结果，（多重采样的渲染结果不可在frag shader中采样，无法进行后处理）
    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 直接渲染到屏幕的四边形
    float screenVertices[] = { // NDC坐标和纹理坐标
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f, // 左上
        -1.0f, -1.0f,  0.0f, 0.0f, // 左下
         1.0f, -1.0f,  1.0f, 0.0f, // 右下

        -1.0f,  1.0f,  0.0f, 1.0f, // 左上
         1.0f, -1.0f,  1.0f, 0.0f, // 右下
         1.0f,  1.0f,  1.0f, 1.0f  // 右上
    };

    unsigned int screenVAO, screenVBO;
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    postProcess_shader.use();
    postProcess_shader.setInt("screenTexture", 0);
    
    // screen_shader.use();
    // screen_shader.setInt("screenTexture", 0);

#pragma endregion

    // 变量
    ImVec4 clear_color{0.2f, 0.3f, 0.3f, 1.0f};
    static bool drawLine = false;
    static int currentEffect = 0; // 后处理效果选择
    postProcess_shader.use();
    postProcess_shader.setInt("effect", currentEffect);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;
        processInput(window);

        if(drawLine)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 渲染fbo
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glEnable(GL_DEPTH_TEST);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.f);

        common_shader.use();  // 激活着色器
        common_shader.setMat4("view", view);
        common_shader.setMat4("projection", projection);
        common_shader.setMat4("model", model);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);

#pragma region 拷贝到另一个帧缓冲
        // 把多重采样FBO的内容解析到中间帧缓冲(同时将多重采样缓冲还原。)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO); 
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#pragma endregion

        // render 屏幕四边形
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST); // 渲染屏幕四边形时不需要深度测试
        glClearColor(1.f,1.f,1.f,1.f); // 不必要，看不到背后

        postProcess_shader.use();
        glBindVertexArray(screenVAO);
        glDrawArrays(GL_TRIANGLES, 0 ,6);
        
        /// --- imgui(最后绘制，避免被覆盖) ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin("Config");

            // 渲染选项
            ImGui::Text("Render Option");
            ImGui::Checkbox("Draw Line Mode", &drawLine);
                
            // 后处理效果
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("后处理效果");

            const char *effects[] = { // 对应frag shader中的枚举
                "无效果",               // 0
                "反色",                 // 1
                "灰度",                 // 2
                "复古",
                "高斯模糊",
                "锐化",
                "边缘检测",
                "sobel 边缘检测X",
                "sobel 边缘检测Y",
                "拉普拉斯边缘检测",
                "浮雕",
                "横向拖影",
                "纵向拖影",
            };
            if(ImGui::Combo("选择效果", &currentEffect, effects, IM_ARRAYSIZE(effects))){
                postProcess_shader.use();
                postProcess_shader.setInt("effect", currentEffect);
            }

            // 其他
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("其他设置");
            ImGui::ColorEdit4("Background Color", (float *)&clear_color);
            ImGui::Text("FPS: %d", int(io.Framerate));

            ImGui::End();
        }
        glClear(GL_DEPTH_BUFFER_BIT);   // 把深度重置到 1.0
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    // 清理资源
    // 其他清理略，漏了一些也无所谓，程序结束了操作系统会回收资源

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}



void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void processInput(GLFWwindow *window)
{
    // 退出
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // 相机移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(DOWN, deltaTime);
    }

    // 光标锁定/解锁切换--> 相机控制 / imgui输入
    static bool tPressedLastFrame = false;
    bool tPressed = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;
    // 只在按键从释放变为按下时触发（边缘检测）
    if (tPressed && !tPressedLastFrame)
    {
        if (cursorLocked)
        { // 解锁光标，用于imgui输入
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 锁定光标，用于控制fps摄像机
            firstMouse = true;                                           // 防止恢复锁定时的位置偏移导致的视角跳动
        }
        cursorLocked = !cursorLocked;
    }
    tPressedLastFrame = tPressed;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (!cursorLocked)
    {                                                         // 光标解锁时不处理相机旋转
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos); // 给 ImGui
        return;
    }
    // 相机旋转
    if (firstMouse)
    {
        firstMouse = false;
        cursorLastX = xpos;
        cursorLastY = ypos;
    }
    double xOffset = xpos - cursorLastX;
    double yOffset = cursorLastY - ypos;
    cursorLastX = xpos;
    cursorLastY = ypos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (!cursorLocked)
    { // 光标解锁时不处理相机，控制imgui
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        return;
    }
    camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!cursorLocked)
    { // 仅光标解锁时允许点击imgui
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        return;
    }
}