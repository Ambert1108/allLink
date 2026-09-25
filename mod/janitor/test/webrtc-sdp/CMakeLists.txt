set(TARGET_NAME "sdpTest")

add_executable(${TARGET_NAME}
 "main.cpp"
  "conductor.cpp"
  "defaults.cpp"
)

target_include_directories( ${TARGET_NAME} 
	PUBLIC ${PROJECT_INCLUDE}
	PRIVATE
    ${WEBRTC_INCLUDE}
	${FFMPEG_INCLUDE_DIR}
	#${OpenCV_INCLUDE_DIRS}
)

target_link_libraries( ${TARGET_NAME}  
	PRIVATE 
		 ${PROJECT_LIBS} 
		 ${WEBRTC_OBJ}
	${FFMPEG_LIBS}
	#${OpenCV_LIBS}
)