#set (TARGET_NAME "janitor")
set (TARGET_NAME "janitor.${PROJECT_VERSION}")



######################################
#  Information
######################################
message(" ===================== Target ${TARGET_NAME} Information ======================")
message(CMAKE_SOURCE_DIR         	    =  ${CMAKE_SOURCE_DIR})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_PREFIX_PATH             =  ${CMAKE_PREFIX_PATH})
message(CMAKE_INCLUDE_PATH            =  ${CMAKE_INCLUDE_PATH})
message(CMAKE_LIBRARY_PATH            =  ${CMAKE_LIBRARY_PATH})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_BUILD_TYPE              =  ${CMAKE_BUILD_TYPE})
message(FFMPEG_INCLUDE_DIR            =  ${FFMPEG_INCLUDE_DIR})
message(FFMPEG_LIBS                   =  ${FFMPEG_LIBS})


add_executable(${TARGET_NAME}
 "main.cpp" 
 "janitor/janitor.cpp"
 "janitor/janitorListener.cpp"

)

target_include_directories( ${TARGET_NAME}  
	PUBLIC ${PROJECT_INCLUDE}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/include
		${FFMPEG_INCLUDE_DIR}
		#${WEBRTC_INCLUDE}

)

target_link_libraries( ${TARGET_NAME}  
	PRIVATE 
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
		${FFMPEG_LIBS}
		${PTHEARD_LIBRARY}
		oatpp::oatpp
		oatpp::oatpp-websocket
	${PROJECT_LIBS}
    #${WEBRTC_LIBS}
	#${WEBRTC_OBJ}
)

message(" ===================== Target ${TARGET_NAME} END ======================")
