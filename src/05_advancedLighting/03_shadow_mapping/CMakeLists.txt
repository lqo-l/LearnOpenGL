add_executable(050301_shadow_mapping 050301_shadow_mapping.cpp)
target_link_libraries(050301_shadow_mapping OpenGLCommon)

add_executable(050302_point_shadow 050302_point_shadow.cpp)
target_link_libraries(050302_point_shadow OpenGLCommon)

file(GLOB shaders "*.vert" "*.frag" "*.geom")
file(COPY ${shaders} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${ASSETS_DIR}/05_advancedLighting/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/05_advancedLighting/)