add_executable(040901_drawLine 040901_drawLine.cpp)
target_link_libraries(040901_drawLine OpenGLCommon)

add_executable(040902_drawTriangle 040902_drawTriangle.cpp)
target_link_libraries(040902_drawTriangle OpenGLCommon)

add_executable(040903_explode 040903_explode.cpp)
target_link_libraries(040903_explode OpenGLCommon)

add_executable(040904_normal_vis 040904_normal_vis.cpp)
target_link_libraries(040904_normal_vis OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag" "*.geom")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)

