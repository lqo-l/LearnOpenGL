cmake_minimum_required(VERSION 3.20)
project(MyLearnOpenGL)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")

#============================编译配置==========================================
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# 定义章节 (目录 开关 描述)
set(CHAPTER_01_beginner_ENABLE OFF)
set(CHAPTER_01_beginner_DESC "入门")

set(CHAPTER_02_lighting_ENABLE OFF)
set(CHAPTER_02_lighting_DESC "光照系统")

set(CHAPTER_03_modelLoading_ENABLE OFF)
set(CHAPTER_03_modelLoading_DESC "模型加载")

set(CHAPTER_04_advanced_ENABLE ON)
set(CHAPTER_04_advanced_DESC "高级OpenGL特性")

# 章节目录列表
set(CHAPTER_DIRS
    01_beginner
    02_lighting
    03_modelLoading
    04_advanced
)

#============================自动添加子目录=====================================
foreach(DIR ${CHAPTER_DIRS})
    option(BUILD_CHAPTER_${DIR} "编译${DIR} - ${CHAPTER_${DIR}_DESC}" ${CHAPTER_${DIR}_ENABLE})
    if(BUILD_CHAPTER_${DIR})
        add_subdirectory(src/${DIR})
        message(STATUS "检测到章节: ${DIR}")
    endif()
endforeach()

#==========================================================================
# # glad（源码库）
# add_library(glad STATIC external/glad/glad.c)
# target_include_directories(glad PUBLIC external)

# glad（预编译库.a）
add_library(glad STATIC IMPORTED)
set_target_properties(glad PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libglad.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/external 
)

# glfw（预编译库.a）
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/external # 头文件也一起设置
)

# # imgui
# file(GLOB IMGUI_SRCS
#     external/imgui/*.cpp
#     external/imgui/backends/imgui_impl_glfw.cpp
#     external/imgui/backends/imgui_impl_opengl3.cpp
# )
# add_library(imgui STATIC ${IMGUI_SRCS})
# target_include_directories(imgui PUBLIC 
#     external/imgui 
#     external/imgui/backends
# )
# target_link_libraries(imgui PUBLIC glfw)  # 前面设置了 glfw 的头文件路径所以不用额外添加
# imgui（预编译库.a）
add_library(imgui STATIC IMPORTED)
set_target_properties(imgui PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libimgui.a
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/imgui;${CMAKE_SOURCE_DIR}/external/imgui/backends"
)

# glm,KHR，stb_image(头文件only，不需要额外处理)

# assimp(.dll+.a导入库)
add_library(assimp SHARED IMPORTED)
set_target_properties(assimp PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libassimp-6.dll 
    IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/lib/libassimp.dll.a 
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/assimp;${CMAKE_SOURCE_DIR}/external/assimp/Compiler;${CMAKE_SOURCE_DIR}/external/assimp/port"
)

# Windows平台需要的系统库
if(WIN32)
    set(LIBS glad glfw imgui assimp opengl32 gdi32 user32)
else()
    set(LIBS glad glfw imgui assimp ${CMAKE_DL_LIBS})
endif()

# 创建接口库,用于子项目继承公共配置
add_library(OpenGLCommon INTERFACE)
target_include_directories(OpenGLCommon INTERFACE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/external/imgui
)
target_link_libraries(OpenGLCommon INTERFACE ${LIBS})
