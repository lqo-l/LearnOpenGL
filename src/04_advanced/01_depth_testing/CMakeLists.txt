add_executable(040101_depth_testing 040101_depth_testing.cpp)
target_link_libraries(040101_depth_testing OpenGLCommon)

add_executable(040102_unlinear_depth 040102_unlinear_depth.cpp)
target_link_libraries(040102_unlinear_depth OpenGLCommon)

file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)