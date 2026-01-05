add_executable(040501_fbo 040501_fbo.cpp)
target_link_libraries(040501_fbo OpenGLCommon)

add_executable(040501_rearview_mirror 040501_rearview_mirror.cpp)
target_link_libraries(040501_rearview_mirror OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)