set(TARGET_NAME "echo")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/echo" CPP_LIST)

add_executable(${TARGET_NAME} 
			"boot.cpp" 
			"controller.cpp" 
			"defaults.cpp" 
#			"render.cpp" 
#			"presenter.cpp" 
#			"renderer.h"
			"renderer.cpp"
#			"videoEngine.cpp"
			"../../include/VideoEngine/MyCapturer.cpp"
			${VideoEngine}
)


target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})