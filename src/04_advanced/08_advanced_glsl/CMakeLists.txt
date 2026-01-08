add_executable(040801_points 040801_points.cpp)
target_link_libraries(040801_points OpenGLCommon)

add_executable(040802_ubo_test 040802_ubo_test.cpp)
target_link_libraries(040802_ubo_test OpenGLCommon)

add_executable(040803_ssbo_test 040803_ssbo_test.cpp)
target_link_libraries(040803_ssbo_test OpenGLCommon)

file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)

