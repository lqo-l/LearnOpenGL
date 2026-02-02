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
void renderScene(const Shader &shader, unsigned int planeVAO, unsigned int cubeVAO);
void renderCube(const Shader &shader, unsigned int cubeVAO, glm::mat4 model);
void renderQuad(const Shader &shader, unsigned int quadVAO);

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
    glDepthFunc(GL_LESS); 

    // 着色器
    Shader simpleDepthShader(getAssetAbsPath(argv[0], "simpleDepth.vert"),getAssetAbsPath(argv[0], "simpleDepth.frag")); // fbo渲染深度贴图
    Shader shader(getAssetAbsPath(argv[0], "phongWithShadow.vert"), getAssetAbsPath(argv[0], "phongWithShadow.frag")); // 主渲染，带阴影
    Shader lightShader(getAssetAbsPath(argv[0], "light.vert"), getAssetAbsPath(argv[0], "light.frag")); // 光源立方体渲染，仅可视化光源位置用
    Shader debugDepthShader(getAssetAbsPath(argv[0], "debugDepth.vert"), getAssetAbsPath(argv[0], "debugDepth.frag")); // 深度贴图可视化

    /// 立方体渲染顶点
	float cubeVertices[] = {
        // positions          // normals           // texture coords
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
        1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

    /// 立方体VAO
	unsigned int cubeVAO, cubeVBO; 
	glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0 * sizeof(float)));
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 平面顶点
    float planeVertices[] = {
        // positions            // normals         // texture Coords .
         15.0f, -0.5f,  15.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        -15.0f, -0.5f,  15.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        -15.0f, -0.5f, -15.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

         15.0f, -0.5f,  15.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        -15.0f, -0.5f, -15.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
         15.0f, -0.5f, -15.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f
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

    // 屏幕四边形
    float quadVertices[] = { // 以三角形带形式绘制 (triangle strip)
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // 加载纹理
    std::string floorTexturePath = getAssetAbsPath(argv[0], "assets/05_advancedLighting/textures/floor.png");
    unsigned int floorTexture = loadTexture(floorTexturePath.c_str());

    // FBO for depth
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024; // 阴影贴图分辨率
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // 超出深度贴图范围的采样使用边界颜色（超出视锥体左右）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // 不需要颜色缓冲，不写入任何颜色缓冲区
    glReadBuffer(GL_NONE); // 不需要颜色缓冲，不从任何颜色缓冲区读取	
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 固定纹理单元访问相同的纹理对象
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    // shader固定参数配置
    shader.use();
    shader.setInt("material.texture_diffuse", 0);
    shader.setInt("shadowMap", 1);

    debugDepthShader.use();
    debugDepthShader.setInt("depthMap", 1);

    //// ---- 可调节变量 ----
    ImVec4 clear_color{0.2f, 0.3f, 0.3f, 1.0f};
    bool debugDepth = true; // 是否显示深度值可视化
    glm::vec3 dirlightPos{-2.0f, 4.0f, -1.0f}; // 定向光源位置
    float lightNearPlane = 1.0f;
    float lightFarPlane = 7.5f;
    bool useCullFrontForDepthMap = true; // 是否在深度贴图生成阶段使用正面剔除,避免使用bias的阴影悬浮
    bool pcfEnabled = true; // 是否开启PCF
    bool adaptiveShadowBias = true; // 是否开启自适应阴影偏差

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
    glm::vec3 pointLightPos = glm::vec3(-2.f, 4.0f, -1.f); 
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


        /// --- depth map pass ---
        simpleDepthShader.use();
        glm::mat4 lightProjection, lightView;
        // 定向光使用orthographic投影矩阵，点光源和聚光灯使用perspective投影矩阵
        if(openParallelLight){ 
            lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, lightNearPlane, lightFarPlane);
            lightView = glm::lookAt(dirlightPos, dirlightPos+parallelLightDirection, glm::vec3(0.f, 1.f, 0.f));
        }else if(openPointLight){
            lightProjection = glm::perspective(glm::radians(90.0f), float(SHADOW_WIDTH) / float(SHADOW_HEIGHT), lightNearPlane, lightFarPlane);
            lightView = glm::lookAt(pointLightPos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
        }
        // else if(openSpotLight){ // 暂不实现聚光灯的阴影映射
        //     lightProjection = glm::perspective(glm::radians(45.0f), float(SHADOW_WIDTH) / float(SHADOW_HEIGHT), lightNearPlane, lightFarPlane);
        //     lightView = glm::lookAt(camera.Position, camera.Position + camera.Front, glm::vec3(0.f, 1.f, 0.f));
        // }
       
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); // 设置视口为深度贴图大小
        if(useCullFrontForDepthMap){
            glCullFace(GL_FRONT); // 使用正面剔除，消除阴影失真的同时不出现阴影悬浮现象
        }
        renderScene(simpleDepthShader, planeVAO, cubeVAO);
        glCullFace(GL_BACK); // 恢复默认设置

        /// --- normal render pass ---
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); 

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.f);
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

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

        // 暂时禁用聚光灯处理
        // shader.setVec3("spot.lightColor", spotLightColor);
        // shader.setVec3("spot.position", camera.Position);
        // shader.setVec3("spot.spotDir", camera.Front); // spotDIr是聚光灯照射方向
        // shader.setFloat("spot.cutOff", glm::cos(glm::radians(cutOffAngle))); // 转为cos，减少片段着色器计算量
        // shader.setFloat("spot.outerCutOff", glm::cos(glm::radians(outerCutOffAngle)));
        // shader.setVec3("spot.Ia", spotIa);
        // shader.setVec3("spot.Id", spotId);
        // shader.setVec3("spot.Is", spotIs);
        // shader.setFloat("spot.constant", spotLightConstant);
        // shader.setFloat("spot.linear", spotLightLinear);
        // shader.setFloat("spot.quadratic", spotLightQuadratic);

        shader.setInt("material.texture_diffuse", 0);
        shader.setFloat("material.texture_specular", texture_specular);
        shader.setFloat("material.shininess", shininess);
        
        renderScene(shader, planeVAO, cubeVAO); // 绘制除了光源可视化之外的场景

        /// 光源Visualize立方体
        if(openParallelLight){
            glm::mat4 model(1.0f);
            model = ModelLookAlong(dirlightPos, parallelLightDirection); // 定向光
            model = glm::scale(model, glm::vec3(0.1f,0.1f,0.2f));           // 缩小一些
            view = camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(camera.Zoom), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.f);

            lightShader.use();
            lightShader.setMat4("view", view);
            lightShader.setMat4("projection", projection);
            renderCube(lightShader, cubeVAO, model);
        }else if(openPointLight){
            glm::mat4 model(1.0f);
            model = glm::translate(model, pointLightPos); // 光源位置
            model = glm::scale(model, glm::vec3(0.1f));           // 缩小一些
            view = camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(camera.Zoom), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 100.f);

            lightShader.use();
            lightShader.setMat4("view", view);
            lightShader.setMat4("projection", projection);
            renderCube(lightShader, cubeVAO, model);
        }
        
        /// debug depth map，在屏幕右下角绘制一个小型的阴影贴图
        if (debugDepth){
            glDisable(GL_DEPTH_TEST); // 关闭深度测试，避免被遮挡
            debugDepthShader.use();
            debugDepthShader.setFloat("near", lightNearPlane);
            debugDepthShader.setFloat("far", lightFarPlane);
            if(openParallelLight){
                debugDepthShader.setBool("needToLinearizeDepth", false); //  定向光是正交投影，深度本身是线性的，不需要线性化
            }else if(openPointLight){
                debugDepthShader.setBool("needToLinearizeDepth", true);
            }
            // 保持阴影贴图纵横比，以高度的1/4为基准
            int debugHeight = SCR_HEIGHT / 4;
            int debugWidth = (int)(debugHeight * ((float)SHADOW_WIDTH / SHADOW_HEIGHT));
            glViewport(SCR_WIDTH - debugWidth - 10, 10, debugWidth, debugHeight);
            renderQuad(debugDepthShader, quadVAO);
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // 恢复视口
            glEnable(GL_DEPTH_TEST);
        }
        
        shader.use();
        shader.setBool("pcfEnabled", pcfEnabled);
        shader.setBool("adaptiveShadowBias", adaptiveShadowBias);

        /// imgui(最后绘制，避免被覆盖)
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
            ImGui::Begin("Config");

            // ImGui::DragFloat("地面镜面反射强度", &texture_specular,0.1, 0.f, 1.f);
            // ImGui::DragFloat("地面镜面shininess", &shininess, 1.f, 2.f, 256.f);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("shadow map设置");
            ImGui::Checkbox("深度值可视化", &debugDepth);
            ImGui::DragFloat("光源视角近裁剪面", &lightNearPlane, 0.1f, 0.1f, 10.f);
            ImGui::DragFloat("光源视角远裁剪面", &lightFarPlane, 0.1f, 10.f, 50.f);
            ImGui::Separator();
            ImGui::Checkbox("开启PCF", &pcfEnabled);
            ImGui::Checkbox("开启自适应阴影偏差", &adaptiveShadowBias);
            ImGui::Checkbox("深度贴图阶段正面剔除", &useCullFrontForDepthMap);
            

            /// 光源
            if (ImGui::CollapsingHeader("平行光设置", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Checkbox("开启平行光", &openParallelLight)) { 
                    if (openParallelLight) openPointLight = false; 
                }
                if (openParallelLight)
                {
                    ImGui::Indent();
                    ImGui::DragFloat3("平行光位置", &dirlightPos[0], 0.1f);
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
                if (ImGui::Checkbox("开启点光源", &openPointLight)) {
                     if (openPointLight) openParallelLight = false; 
                }
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

            // 暂时禁用聚光灯的处理
            // if (ImGui::CollapsingHeader("聚光灯设置", ImGuiTreeNodeFlags_DefaultOpen))
            // {
            //     ImGui::Checkbox("开启聚光灯", &openSpotLight);
            //     if (openSpotLight)
            //     {
            //         ImGui::Indent();
            //         ImGui::ColorEdit3("聚光灯颜色", &spotLightColor[0]);
            //         ImGui::DragFloat("内切角", &cutOffAngle, 0.1f, 0.f, outerCutOffAngle);
            //         ImGui::DragFloat("外切角", &outerCutOffAngle, 0.1f, cutOffAngle, 90.f);
            //         ImGui::ColorEdit3("聚光Ia", &spotIa[0]);
            //         ImGui::ColorEdit3("聚光Id", &spotId[0]);
            //         ImGui::ColorEdit3("聚光Is", &spotIs[0]);
            //         ImGui::DragFloat("聚光衰减常数项", &spotLightConstant, 0.1f, 0.f, 5.f);
            //         ImGui::DragFloat("聚光衰减线性项", &spotLightLinear, 0.001f, 0.f, 1.f);
            //         ImGui::DragFloat("聚光衰减二次项", &spotLightQuadratic, 0.001f, 0.f, 2.f);
            //         ImGui::Unindent();
            //     }
            // }

            // 其他
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("其他设置");
            ImGui::ColorEdit4("Background Color", (float *)&clear_color);
            ImGui::Text("FPS: %d", int(io.Framerate));
            ImGui::Text("Camera Speed: %.1f", camera.MovementSpeed);

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

// renders the 3D scene
// 两次调用，第一次绘制阴影贴图，第二次绘制场景
void renderScene(const Shader &shader, unsigned int planeVAO, unsigned int cubeVAO)
{
    // floor
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // 随便写一些cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    renderCube(shader, cubeVAO, model);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
    renderCube(shader, cubeVAO, model);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.25));
    renderCube(shader, cubeVAO, model);
}

void renderCube(const Shader &shader, unsigned int cubeVAO, glm::mat4 model){
    // render Cube
    shader.use();
    shader.setMat4("model", model);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderQuad(const Shader &shader, unsigned int quadVAO){
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}



// ------------ 回调和输入处理函数 ---------------

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

    // 上下箭头增减相机移动速度
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        camera.MovementSpeed += 5.0f * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        camera.MovementSpeed -= 5.0f * deltaTime;
        if (camera.MovementSpeed < 0.0f)
            camera.MovementSpeed = 0.0f;
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