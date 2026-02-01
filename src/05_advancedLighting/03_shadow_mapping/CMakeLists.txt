add_executable(050301_shadow_mapping 050301_shadow_mapping.cpp)
target_link_libraries(050301_shadow_mapping OpenGLCommon)


file(GLOB shaders "*.vert" "*.frag")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/05_advancedLighting/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/05_advancedLighting/)