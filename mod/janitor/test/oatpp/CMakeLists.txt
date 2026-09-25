set(TARGET_NAME "oatpp")
file(GLOB_RECURSE CPP_LIST "${PROJECT_SOURCE_DIR}/test/oatpp/*.cpp")

add_executable(${TARGET_NAME} ${CPP_LIST})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})

#file(COPY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources/")