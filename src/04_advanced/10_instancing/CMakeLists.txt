add_executable(041001_uniform_instancing 041001_uniform_instancing.cpp)
target_link_libraries(041001_uniform_instancing OpenGLCommon)

add_executable(041002_vbo_instancing 041002_vbo_instancing.cpp)
target_link_libraries(041002_vbo_instancing OpenGLCommon)

add_executable(041003_asteroid_belt 041003_asteroid_belt.cpp)
target_link_libraries(041003_asteroid_belt OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag" "*.geom")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)

# dll
file(COPY ${CMAKE_SOURCE_DIR}/lib/libassimp-6.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
