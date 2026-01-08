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

    // 着色器(两端shader程序,使用不同的model和相同的view,proj,测试ubo共享)
    Shader shader1(getAssetAbsPath(argv[0], "ssbo_test.vert"), getAssetAbsPath(argv[0], "ssbo_test.frag")); 
  

    // 立方体顶点、uv坐标、法线。(x,  y,  z,  u, v,  nx, ny, nz, )
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
    };


    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)0);
    glBindVertexArray(0);

#pragma region ssbo
    // 创建ssbo
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    // 获取shader中的block idx
    GLuint index = glGetProgramResourceIndex(shader1.ID, GL_SHADER_STORAGE_BLOCK, "testBlock");
    // 获取block size
    GLint blockSize = 0;
    GLenum prop = GL_BUFFER_DATA_SIZE;
    glGetProgramResourceiv(
        shader1.ID, 
        GL_SHADER_STORAGE_BLOCK, 
        index,
        1,      // 属性数量
        &prop,  // 属性数组
        1,      // 返回值数量上限
        NULL,   // 实际返回数量（可选）
        &blockSize // 输出结果
    );

    // 申请内存
    glBufferData(GL_SHADER_STORAGE_BUFFER, blockSize, NULL, GL_DYNAMIC_DRAW); // 申请block大小的空间
    // ssbo绑定绑定点
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo); // 绑定到绑定点1
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // shader buffer块绑定到绑定点
    // glShaderStorageBlockBinding(shader1.ID, index, 1); (shader中已经指定了，这里注释掉)

    ///  获取每个变量的偏移
    // 获取变量个数
    GLint numMembers;
    glGetProgramInterfaceiv(
        shader1.ID,
        GL_BUFFER_VARIABLE,      // ← 查询 buffer 内部的变量（即成员）
        GL_ACTIVE_RESOURCES,
        &numMembers
    );  
    std::unordered_map<std::string, GLint> memberOffsets; // 变量名，偏移量
    for (GLuint i = 0; i < numMembers; ++i) { // OpenGL 不保证 GL_BUFFER_VARIABLE 的枚举顺序
        // 获取成员名称（可选）
        char name[256];
        GLsizei length;
        glGetProgramResourceName(
            shader1.ID,
            GL_BUFFER_VARIABLE,
            i,
            sizeof(name),
            &length,
            name
        );

        // 查询该成员的 offset
        GLenum prop = GL_OFFSET;
        GLint offset = -1;
        glGetProgramResourceiv(
            shader1.ID,
            GL_BUFFER_VARIABLE,   // ← 关键：查的是 buffer variable（SSBO/UBO 成员）
            i,                    // ← 成员的 resource index（从 0 到 numMembers-1）
            1,
            &prop,
            1,
            NULL,
            &offset
        );
        memberOffsets[name] = offset;
        printf("Member '%s' offset = %d\n", name, offset);
    }

    // 注：这里想通过变量位置直接设置测试写的变量 write_color_in_shader
    // 但是实测发现idx = 3的位置是view， idx=2的地方是projection，似乎和定义顺序反过来了。所以通过上一步的哈希表获取变量名对应offset
    glm::vec3 write_color_in_shader{}; 
    // prop = GL_OFFSET;
    // GLint Idx3_Offset = -1;
    // glGetProgramResourceiv(
    //     shader1.ID,
    //     GL_BUFFER_VARIABLE,   // 查的是 buffer variable（SSBO/UBO 成员）
    //     3,                    // 原以为3是write_color_in_shader成员的index，结果不是
    //     1,
    //     &prop,
    //     1,
    //     NULL,
    //     &Idx3_Offset
    // );
    // std::cout<<"Idx3_Offset: "<< Idx3_Offset <<std::endl;
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, memberOffsets["write_color_in_shader"], sizeof(glm::vec3), &write_color_in_shader);  // 更新is_color_black,glsl bool是4B

    // GLint Idx2_Offset = -1;
    // glGetProgramResourceiv(
    //     shader1.ID,
    //     GL_BUFFER_VARIABLE,   // 查的是 buffer variable（SSBO/UBO 成员）
    //     2,                    // 原以为2是color成员的index
    //     1,
    //     &prop,
    //     1,
    //     NULL,
    //     &Idx2_Offset
    // );
    // std::cout<<"Idx2_Offset: "<< Idx2_Offset <<std::endl;
    

#pragma endregion ssbo


    // 变量
    ImVec4 clear_color{0.2f, 0.3f, 0.3f, 1.0f};
    static glm::vec3 color{1.0f, 0.0f, 0.0f};
    glEnable(GL_DEPTH_TEST);
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;
        processInput(window);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region ssbo update
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.f);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));  // 更新view , sizeof(glm::mat4) = 16*4 = 64字节,可以直接写64
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));  // 更新proj
        // is_color_black在gpu中修改，测试用，不在cpu中设置
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, memberOffsets["color"], sizeof(glm::vec3), glm::value_ptr(color));  // 更新color

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

#pragma endregion ssbo update
        
        // shader1
        shader1.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
        shader1.setMat4("model", model);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

#pragma region read ssbo back
        // cpu获取数据 
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);  // 必须先绑定！
        // 直接get
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  
            // GL_BUFFER_UPDATE_BARRIER_BIT	 用于buffer等cpu写入
            // GL_SHADER_STORAGE_BARRIER_BIT	用于cpu等gpu写入
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, memberOffsets["write_color_in_shader"], sizeof(glm::vec3), &write_color_in_shader); // 直接获取buffer块数据到结构体

        // // 映射方式
        // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);//隔断作用，为了让数据修改完成
        // void* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY); //获取着色器buffer块内存地址
        // glm::vec3* write_color_in_shader_in_block = (glm::vec3*)((char*)p + memberOffsets["write_color_in_shader"]);
        // memcpy(&write_color_in_shader, (void*)write_color_in_shader_in_block, sizeof(glm::vec3));//拷贝buffer块数据到结构体
        // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

#pragma endregion read ssbo back

        /// --- imgui(最后绘制，避免被覆盖) ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin("Config");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::SliderFloat3("Cube Color", (float*)&color, 0.0f, 1.0f);
            ImGui::Text("write_color_in_shader: (%.2f, %.2f, %.2f)", write_color_in_shader.x, write_color_in_shader.y, write_color_in_shader.z);


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
    // 清理资源(略，漏了一些也无所谓，这里程序结束了操作系统会回收资源)

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