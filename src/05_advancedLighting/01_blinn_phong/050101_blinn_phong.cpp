#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

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

Camera camera(glm::vec3(0.f, 5.f, 10.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

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

    // opengl 深度状态
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS); // always pass the depth test (same effect as glDisable(GL_DEPTH_TEST))

    // 着色器
    Shader shader(getAssetAbsPath(argv[0], "phong_world.vert"), getAssetAbsPath(argv[0], "phong_world.frag"));
    Shader lightShader(getAssetAbsPath(argv[0], "light.vert"), getAssetAbsPath(argv[0], "light.frag"));

    /// 立方体渲染顶点
	float cubeVertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
	};

    /// 光源立方体顶点VAO，想独立于物体
	unsigned int lightVAO, VBO;
	glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &VBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

    // 平面顶点
    float planeVertices[] = {
        // positions            // normals         // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  2.0f, 2.0f
    };
    
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // 加载纹理
    std::string floorTexturePath = getAssetAbsPath(argv[0], "assets/05_advancedLighting/textures/floor.png");
    unsigned int floorTexture = loadTexture(floorTexturePath.c_str());
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);


    // 变量
    ImVec4 clear_color{0.2f, 0.3f, 0.3f, 1.0f};
    static bool use_blinn_phong = true;
    shader.use();
    shader.setBool("useBlinnPhong", use_blinn_phong);

    // 光源开启设置
    bool openParallelLight = false;
    bool openPointLight = true;
    bool openSpotLight = false;

    // 平行光
    glm::vec3 parallelLightColor = glm::vec3(1.f);
    glm::vec3 parallelLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f); 
    glm::vec3 parallelIa = glm::vec3(0.2f);
    glm::vec3 parallelId = glm::vec3(0.5f);
    glm::vec3 parallelIs = glm::vec3(0.2f);

    // 点光源
    glm::vec3 pointLightColor = glm::vec3(1.f);
    glm::vec3 pointLightPos = glm::vec3(-0.2f, 1.0f, -0.3f); 
    float pointLightConstant = 1.0f;
    float pointLightLinear = 0.09f;
    float pointLightQuadratic = 0.032f;
    glm::vec3 pointIa = glm::vec3(0.2f);
    glm::vec3 pointId = glm::vec3(0.6f);
    glm::vec3 pointIs = glm::vec3(0.6f);

    // 聚光灯
    glm::vec3 spotLightColor = glm::vec3(1.f);
    float spotLightConstant = 1.0f;
    float spotLightLinear = 0.02f;
    float spotLightQuadratic = 0.006f;
    float cutOffAngle = 4.f;
    float outerCutOffAngle = 8.f; 
    glm::vec3 spotIa = glm::vec3(0.1f);
    glm::vec3 spotId = glm::vec3(0.6f);
    glm::vec3 spotIs = glm::vec3(0.6f);

    // 地板texture_specular
    float texture_specular = 0.8f;
    float shininess = 32.f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;
        processInput(window);

        // render
        // ------
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// plane
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.f);
        glm::mat4 model = glm::mat4(1.f);
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("model", model);

        shader.setVec3("viewPos", camera.Position);
        shader.setBool("openParallelLight", openParallelLight);
        shader.setBool("openPointLight", openPointLight);
        shader.setBool("openSpotLight", openSpotLight);

        shader.setVec3("parallel.direction", parallelLightDirection);
        shader.setVec3("parallel.lightColor", parallelLightColor);
        shader.setVec3("parallel.Ia", parallelIa);
        shader.setVec3("parallel.Id", parallelId);
        shader.setVec3("parallel.Is", parallelIs);

        shader.setVec3("point.lightColor", pointLightColor);
        shader.setVec3("point.position", pointLightPos);
        shader.setVec3("point.Ia", pointIa);
        shader.setVec3("point.Id", pointId);
        shader.setVec3("point.Is", pointIs);
        shader.setFloat("point.constant", pointLightConstant);
        shader.setFloat("point.linear", pointLightLinear);
        shader.setFloat("point.quadratic", pointLightQuadratic);

        shader.setVec3("spot.lightColor", spotLightColor);
        shader.setVec3("spot.position", camera.Position);
        shader.setVec3("spot.spotDir", camera.Front); // spotDIr是聚光灯照射方向
        shader.setFloat("spot.cutOff", glm::cos(glm::radians(cutOffAngle))); // 转为cos，减少片段着色器计算量
        shader.setFloat("spot.outerCutOff", glm::cos(glm::radians(outerCutOffAngle)));
        shader.setVec3("spot.Ia", spotIa);
        shader.setVec3("spot.Id", spotId);
        shader.setVec3("spot.Is", spotIs);
        shader.setFloat("spot.constant", spotLightConstant);
        shader.setFloat("spot.linear", spotLightLinear);
        shader.setFloat("spot.quadratic", spotLightQuadratic);

        shader.setInt("material.texture_diffuse", 0);
        shader.setFloat("material.texture_specular", texture_specular);
        shader.setFloat("material.shininess", shininess);
        
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        /// 光源
        model = ModelLookAt(pointLightPos, {0.f, 0.f, 0.f}); // 相机看向原点
        model = glm::scale(model, glm::vec3(0.2f));           // 缩小一些
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.f);

        lightShader.use();
        lightShader.setMat4("model", model);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        /// imgui(最后绘制，避免被覆盖)
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin("Config");

            if(ImGui::Checkbox("Use Blinn-Phong Model", &use_blinn_phong)){
                shader.use();
                shader.setBool("useBlinnPhong", use_blinn_phong);
            }
            ImGui::DragFloat("地面镜面反射强度", &texture_specular,0.1, 0.f, 1.f);
            ImGui::DragFloat("地面镜面shininess", &shininess, 1.f, 2.f, 256.f);

            /// 光源
            if (ImGui::CollapsingHeader("平行光设置", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("开启平行光", &openParallelLight);
                if (openParallelLight)
                {
                    ImGui::Indent();
                    ImGui::ColorEdit3("平行光颜色", &parallelLightColor[0]);
                    ImGui::DragFloat3("平行光方向", &parallelLightDirection[0], 0.1f);
                    ImGui::ColorEdit3("平行光Ia", &parallelIa[0]);
                    ImGui::ColorEdit3("平行光Id", &parallelId[0]);
                    ImGui::ColorEdit3("平行光Is", &parallelIs[0]);
                    ImGui::Unindent();
                }
            }

            if (ImGui::CollapsingHeader("点光源设置", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("开启点光源", &openPointLight);
                if (openPointLight)
                {
                    ImGui::Indent();
                    ImGui::ColorEdit3("点光源颜色", &pointLightColor[0]);
                    ImGui::DragFloat3("点光源位置", &pointLightPos[0], 0.1f);
                    ImGui::ColorEdit3("点光源Ia", &pointIa[0]);
                    ImGui::ColorEdit3("点光源Id", &pointId[0]);
                    ImGui::ColorEdit3("点光源Is", &pointIs[0]);
                    ImGui::DragFloat("点光源衰减常数项", &pointLightConstant, 0.1f, 0.f, 5.f);
                    ImGui::DragFloat("点光源衰减线性项", &pointLightLinear, 0.001f, 0.f, 1.f);
                    ImGui::DragFloat("点光源衰减二次项", &pointLightQuadratic, 0.001f, 0.f, 2.f);
                    ImGui::Unindent();
                }
            }

            if (ImGui::CollapsingHeader("聚光灯设置", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("开启聚光灯", &openSpotLight);
                if (openSpotLight)
                {
                    ImGui::Indent();
                    ImGui::ColorEdit3("聚光灯颜色", &spotLightColor[0]);
                    ImGui::DragFloat("内切角", &cutOffAngle, 0.1f, 0.f, outerCutOffAngle);
                    ImGui::DragFloat("外切角", &outerCutOffAngle, 0.1f, cutOffAngle, 90.f);
                    ImGui::ColorEdit3("聚光Ia", &spotIa[0]);
                    ImGui::ColorEdit3("聚光Id", &spotId[0]);
                    ImGui::ColorEdit3("聚光Is", &spotIs[0]);
                    ImGui::DragFloat("聚光衰减常数项", &spotLightConstant, 0.1f, 0.f, 5.f);
                    ImGui::DragFloat("聚光衰减线性项", &spotLightLinear, 0.001f, 0.f, 1.f);
                    ImGui::DragFloat("聚光衰减二次项", &spotLightQuadratic, 0.001f, 0.f, 2.f);
                    ImGui::Unindent();
                }
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