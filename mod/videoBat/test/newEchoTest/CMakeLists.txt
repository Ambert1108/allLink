set(TARGET_NAME "echo")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/newEchoTest" CPP_LIST)
add_executable(${TARGET_NAME}
        ${CPP_LIST}
        ${PROJECT_SOURCE_DIR}/include/engine/rtcVideoEngine.cpp
)

target_include_directories("echo"
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_INCLUDE}
        ${FFMPEG_INCLUDE_DIR}
	    ${OpenCV_INCLUDE_DIRS}
)


target_link_libraries("echo"
        PRIVATE
        ${PROJECT_LIBS}
        ${FFMPEG_LIBS}
	    ${OpenCV_LIBS}
)

