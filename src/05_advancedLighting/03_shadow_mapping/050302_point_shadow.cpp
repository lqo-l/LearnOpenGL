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
void renderScene(const Shader &shader, unsigned int cubeVAO);
void renderCube(const Shader &shader, unsigned int cubeVAO, glm::mat4 model);
void renderQuad(const Shader &shader, unsigned int quadVAO);

unsigned int SCR_WIDTH = 1440;
unsigned int SCR_HEIGHT = 800;
bool cursorLocked = true;

Camera camera(glm::vec3(0.f, 5.f, 7.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

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
    glEnable(GL_CULL_FACE);

    // 着色器
    Shader cubeDepthShader(getAssetAbsPath(argv[0], "02_cubeDepth.vert"), getAssetAbsPath(argv[0], "02_cubeDepth.frag"), getAssetAbsPath(argv[0], "02_cubeDepth.geom")); // fbo渲染深度贴图 (包含geometry shader)
    Shader shader(getAssetAbsPath(argv[0], "02_phongWithShadow.vert"), getAssetAbsPath(argv[0], "02_phongWithShadow.frag")); // 主渲染，带阴影
    Shader lightShader(getAssetAbsPath(argv[0], "light.vert"), getAssetAbsPath(argv[0], "light.frag")); // 光源立方体渲染，仅可视化光源位置用

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

#pragma region 立方体贴图
    // 立方体贴图
    GLuint depthCubemap;
    glGenTextures(1, &depthCubemap);
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024; // 阴影贴图分辨率
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (GLuint i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    // FBO for depth
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    
    /// 每次绑定深度贴图不同的目标，渲染6次的写法
    // for(int i = 0; i < 6; i++)
    // {
    //     GLuint face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, depthCubemap, 0);
    //     BindViewMatrix(lightViewMatrices[i]);
    //     RenderScene();  
    // }

    /// 一次性绑定深度贴图，一次渲染中写完六个深度贴图
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE); // 不需要颜色缓冲，不写入任何颜色缓冲区
    glReadBuffer(GL_NONE); // 不需要颜色缓冲，不从任何颜色缓冲区读取	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion 立方体贴图  

    // 固定纹理单元访问相同的纹理对象
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap); // 深度立方体贴图

    // shader固定参数配置
    shader.use();
    shader.setInt("material.texture_diffuse", 0);
    shader.setInt("depthCubeMap", 1);

    //// ---- 可调节变量 ----
    ImVec4 clear_color{0.2f, 0.3f, 0.3f, 1.0f};
    float lightNearPlane = 1.0f;
    float lightFarPlane = 25.0f;
    bool useCullFrontForDepthMap = true; // 是否在深度贴图生成阶段使用正面剔除,避免使用bias的阴影悬浮
    bool pcfEnabled = true; // 是否开启PCF
    bool adaptiveShadowBias = true; // 是否开启自适应阴影偏差
    bool OptimizedPcf = true; // 优化pcf：从垂直方向采样
    bool debugShadowMap = false; // 调试阴影贴图
    shader.use();
    shader.setBool("debugShadowMap", debugShadowMap);

    // 光源开启设置
    bool openPointLight = true;

    // 点光源
    glm::vec3 pointLightColor = glm::vec3(1.f);
    glm::vec3 pointLightPos = glm::vec3(0.f, 0.0f, 0.f); 
    float pointLightConstant = 1.0f;
    float pointLightLinear = 0.09f;
    float pointLightQuadratic = 0.032f;
    glm::vec3 pointIa = glm::vec3(0.2f);
    glm::vec3 pointId = glm::vec3(1.f);
    glm::vec3 pointIs = glm::vec3(0.6f);


    // 地板texture_specular
    float texture_specular = 0.8f;
    float shininess = 32.f;

#pragma region 主循环
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;

        glfwPollEvents();
        processInput(window);


        /// --- depth map pass ---
        GLfloat aspect = (GLfloat)SHADOW_WIDTH/(GLfloat)SHADOW_HEIGHT;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, lightNearPlane, lightFarPlane); // 90度确保覆盖立方体每个面
        // 六个视图矩阵,注视着：右、左、上、下、近、远
        std::vector<glm::mat4> shadowTransforms; 
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(pointLightPos, pointLightPos + glm::vec3(1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(pointLightPos, pointLightPos + glm::vec3(-1.0,0.0,0.0), glm::vec3(0.0,-1.0,0.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0,1.0,0.0), glm::vec3(0.0,0.0,1.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0,-1.0,0.0), glm::vec3(0.0,0.0,-1.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,-1.0,0.0)));
        shadowTransforms.push_back(shadowProj * 
                        glm::lookAt(pointLightPos, pointLightPos + glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,-1.0,0.0)));

        // shader参数
        cubeDepthShader.use();
        for(int i = 0; i < 6; ++i){
            std::string uniformName = "shadowMatrices[" + std::to_string(i) + "]";
            cubeDepthShader.setMat4(uniformName.c_str(), shadowTransforms[i]);
        }
        cubeDepthShader.setVec3("lightPos", pointLightPos);
        cubeDepthShader.setFloat("far_plane", lightFarPlane);

        // 渲染场景到深度立方体贴图
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); // 设置视口为深度贴图大小
        if(useCullFrontForDepthMap){
            glCullFace(GL_FRONT); // 使用正面剔除，消除阴影失真的同时不出现阴影悬浮现象
        }
        renderScene(cubeDepthShader, cubeVAO);
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
        shader.setFloat("far_plane", lightFarPlane);        

        shader.setVec3("viewPos", camera.Position);

        shader.setBool("openPointLight", openPointLight);

        shader.setVec3("point.lightColor", pointLightColor);
        shader.setVec3("point.position", pointLightPos);
        shader.setVec3("point.Ia", pointIa);
        shader.setVec3("point.Id", pointId);
        shader.setVec3("point.Is", pointIs);
        shader.setFloat("point.constant", pointLightConstant);
        shader.setFloat("point.linear", pointLightLinear);
        shader.setFloat("point.quadratic", pointLightQuadratic);

        shader.setInt("material.texture_diffuse", 0);
        shader.setFloat("material.texture_specular", texture_specular);
        shader.setFloat("material.shininess", shininess);
        
        renderScene(shader, cubeVAO); // 绘制除了光源可视化之外的场景

        /// 光源Visualize立方体
        if(openPointLight){
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
        
        shader.use();
        shader.setBool("pcfEnabled", pcfEnabled);
        shader.setBool("adaptiveShadowBias", adaptiveShadowBias);
        shader.setBool("OptimizedPcf", OptimizedPcf);

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
            if(ImGui::Checkbox("调试：Depth画面", &debugShadowMap)){shader.use();shader.setBool("debugShadowMap", debugShadowMap);};
            ImGui::Separator();
            ImGui::DragFloat("光源视角近裁剪面", &lightNearPlane, 0.1f, 0.1f, 10.f);
            ImGui::DragFloat("光源视角远裁剪面", &lightFarPlane, 0.1f, 10.f, 50.f);
            ImGui::Separator();
            ImGui::Checkbox("开启自适应阴影偏差", &adaptiveShadowBias);
            if(ImGui::Checkbox("开启PCF", &pcfEnabled)){
                if(!pcfEnabled){
                    OptimizedPcf = false;
                }
            }
            if(ImGui::Checkbox("优化自适应阴影偏差(仅垂直方向采样)", &OptimizedPcf)){
                if(OptimizedPcf){
                    pcfEnabled = true;
                }
            } // 强制开启自适应阴影偏差
            ImGui::Checkbox("深度贴图阶段正面Cull", &useCullFrontForDepthMap);
            

            /// 光源
            if (ImGui::CollapsingHeader("点光源设置", ImGuiTreeNodeFlags_DefaultOpen))
            {
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
void renderScene(const Shader &shader, unsigned int cubeVAO)
{
    // Room Cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(10.0));
    shader.setBool("reverse_normals",true);
    glDisable(GL_CULL_FACE);        // 由于立方体是从内部看的，所以关闭剔除，否则内部立方体就被剔除了
    renderCube(shader, cubeVAO, model);
    shader.setBool("reverse_normals",false);
    glEnable(GL_CULL_FACE);

    // 随便写一些cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
    renderCube(shader, cubeVAO, model);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
    model = glm::scale(model, glm::vec3(1.5));
    renderCube(shader, cubeVAO, model);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
    renderCube(shader, cubeVAO, model);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
    renderCube(shader, cubeVAO, model);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    model = glm::rotate(model, 60.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(1.5));
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