set(TARGET_NAME "retTest")
add_executable(${TARGET_NAME} "echoTest.cpp")


target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})