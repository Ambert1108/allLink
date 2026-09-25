set(TARGET_NAME "echoApi")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/echoApiTest" CPP_LIST)
add_executable(${TARGET_NAME}
        ${CPP_LIST}
        ${PROJECT_SOURCE_DIR}/include/engine/rtcVideoEngine.cpp
)

target_include_directories("echoApi"
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_INCLUDE}
        ${FFMPEG_INCLUDE_DIR}
	    ${OpenCV_INCLUDE_DIRS}
)


target_link_libraries("echoApi"
        PRIVATE
        ${PROJECT_LIBS}
        ${FFMPEG_LIBS}
	    ${OpenCV_LIBS}
        dwmapi
        D3D11
        Shcore
        DXGI
)

