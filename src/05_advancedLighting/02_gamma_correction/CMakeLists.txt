add_executable(050201_gamma_correction 050201_gamma_correction.cpp)
target_link_libraries(050201_gamma_correction OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/05_advancedLighting/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/05_advancedLighting/)