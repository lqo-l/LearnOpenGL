add_executable(040601_cubemaps 040601_cubemaps.cpp)
target_link_libraries(040601_cubemaps OpenGLCommon)

add_executable(040602_cubemaps_optim 040602_cubemaps_optim.cpp)
target_link_libraries(040602_cubemaps_optim OpenGLCommon)

add_executable(040603_env_map 040603_env_map.cpp)
target_link_libraries(040603_env_map OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)

# dll
file(COPY ${CMAKE_SOURCE_DIR}/lib/libassimp-6.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})