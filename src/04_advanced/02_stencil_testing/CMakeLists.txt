add_executable(040201_stencil_testing 040201_stencil_testing.cpp)
target_link_libraries(040201_stencil_testing OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)