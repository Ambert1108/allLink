set(TARGET_NAME "screenCapture")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/screenCapture" CPP_LIST)

add_executable(${TARGET_NAME} 
			"boot.cpp" 
			"controller.cpp" 
			"defaults.cpp" 
#			"render.cpp" 
#			"presenter.cpp" 
#			"renderer.h"
			"renderer.cpp"
			"../../include/engine/rtcVideoEngine.cpp"
			"../../include/engine/screenCapturer.cpp"
			${VideoEngine}
)


target_include_directories(${TARGET_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_INCLUDE}
)
target_link_libraries(${TARGET_NAME} PRIVATE 
	${PROJECT_LIBS}
	${WEBRTC_OBJ}
    dwmapi
    D3D11
    Shcore
    DXGI
)