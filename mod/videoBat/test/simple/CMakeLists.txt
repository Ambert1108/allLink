#set(TARGET_NAME "simple")
#aux_source_directory("${PROJECT_SOURCE_DIR}/test/simple" CPP_LIST)
#add_executable(${TARGET_NAME} ${CPP_LIST})
#
#target_include_directories( ${TARGET_NAME}  
#	PRIVATE
#		${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/include
#		${PROJECT_SOURCE_DIR}/include
#    ${WEBRTC_INCLUDE}
#	${FFMPEG_INCLUDE_DIR}
#	${OpenCV_INCLUDE_DIRS}
#)
#
#target_link_libraries( ${TARGET_NAME}  
#	PRIVATE 
#		${CMAKE_THREAD_LIBS_INIT}
#		${CMAKE_DL_LIBS}
#    ${WEBRTC_LIBS}
#	${WEBRTC_OBJ}
#	${FFMPEG_LIBS}
#	${OpenCV_LIBS}
#)


set(TARGET_NAME "SDLtest")
aux_source_directory("${PROJECT_SOURCE_DIR}/test/simple" CPP_LIST)
add_executable(${TARGET_NAME} ${CPP_LIST})

target_include_directories("SDLtest"
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_INCLUDE}
        ${FFMPEG_INCLUDE_DIR}
	    ${OpenCV_INCLUDE_DIRS}
)


target_link_libraries("SDLtest"
        PRIVATE
        ${PROJECT_LIBS}
        ${FFMPEG_LIBS}
	    ${OpenCV_LIBS}
)

