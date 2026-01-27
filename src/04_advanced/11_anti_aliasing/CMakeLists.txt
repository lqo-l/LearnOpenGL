add_executable(041101_msaa 041101_msaa.cpp)
target_link_libraries(041101_msaa OpenGLCommon)

add_executable(041102_off_screen_msaa 041102_off_screen_msaa.cpp)
target_link_libraries(041102_off_screen_msaa OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag" "*.geom")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)

