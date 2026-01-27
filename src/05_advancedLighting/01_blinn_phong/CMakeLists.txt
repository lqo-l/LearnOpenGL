add_executable(050101_blinn_phong 050101_blinn_phong.cpp)
target_link_libraries(050101_blinn_phong OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/05_advancedLighting/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/05_advancedLighting/)