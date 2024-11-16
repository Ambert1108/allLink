set(TEST_NAME "game1")

add_executable(${TEST_NAME} "main.cpp")

target_include_directories(${TEST_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TEST_NAME} PUBLIC ${PROJECT_LIBS} ${WEBRTC_OBJ})

#TODO：拷贝配置文件与素材
#file(COPY "${PROJECT_SOURCE_DIR}/resource/config.ini" 
#		 DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
#
#file(COPY "${PROJECT_SOURCE_DIR}/resource/Config" 
#		 DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
#
if(EXISTS "${PROJECT_SOURCE_DIR}/resources/" AND NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/resources/")
	file(COPY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources/")
endif()