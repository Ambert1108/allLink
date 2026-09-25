set(TARGET_NAME "audioTest")
add_executable(${TARGET_NAME}
 "main.cpp")
target_include_directories( ${TARGET_NAME}  
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/include
    ${WEBRTC_INCLUDE}
	${FFMPEG_INCLUDE_DIR}
	${OpenCV_INCLUDE_DIRS}
)

target_link_libraries( ${TARGET_NAME}  
	PRIVATE 
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
    ${WEBRTC_LIBS}
	${WEBRTC_OBJ}
	${FFMPEG_LIBS}
	${OpenCV_LIBS}
)