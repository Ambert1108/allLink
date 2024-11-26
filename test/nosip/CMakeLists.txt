set(TARGET_NAME "nosip")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/nosip" CPP_LIST)

add_executable(${TARGET_NAME} "test.cpp" "wslistener.h" "wslistener.cpp")

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})

#file(COPY "${PROJECT_SOURCE_DIR}/res/config.ini" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources/")