set(TARGET_NAME "testSIP")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/testSIP" CPP_LIST)

add_executable(${TARGET_NAME} "audio.cpp")

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})

file(COPY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources/")