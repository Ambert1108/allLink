set(TARGET_NAME "localCap")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/localCapture" CPP_LIST)

add_executable(${TARGET_NAME} ${CPP_LIST})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})

#file(COPY "${PROJECT_SOURCE_DIR}/res/config.ini" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources/")