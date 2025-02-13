set(TARGET_NAME "AllLink.${PROJECT_VERSION}")
file(GLOB_RECURSE CPP_LIST "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE MOD_LIST "${PROJECT_SOURCE_DIR}/core/engine/*.cpp")

add_executable(${TARGET_NAME} ${CPP_LIST} ${MOD_LIST} logo.rc)

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ} d3d11.lib dwmapi.lib dxgi.lib Shcore.lib)

file(COPY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources/")