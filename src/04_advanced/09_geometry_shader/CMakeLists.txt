add_executable(040901_drawLine 040901_drawLine.cpp)
target_link_libraries(040901_drawLine OpenGLCommon)

add_executable(040902_drawTriangle 040902_drawTriangle.cpp)
target_link_libraries(040902_drawTriangle OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag" "*.geom")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/04_advanced/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/04_advanced/)

