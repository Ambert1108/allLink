#set(TARGET_NAME "webrtcTrace.${PROJECT_VERSION}")
#aux_source_directory("${PROJECT_SOURCE_DIR}/src" CPP_LIST)
#option(IS_WIN32 "is win32 object" OFF)
#
#if(${ObjType} MATCHES "win32")
#	add_executable(${TARGET_NAME} WIN32 ${CPP_LIST})
#	target_compile_definitions(${TARGET_NAME} PRIVATE IS_WIN32=1)
#	else()
#	add_executable(${TARGET_NAME} ${CPP_LIST})
#	target_compile_definitions(${TARGET_NAME} PRIVATE IS_WIN32=0)
#	endif()
#
#target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
#target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})

#file(COPY "${PROJECT_SOURCE_DIR}/res/config.ini" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources/")

set(TARGET_NAME "webrtcTrace")
list(APPEND PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/test/trace)
aux_source_directory("${PROJECT_SOURCE_DIR}/test/trace" CPP_LIST)

add_executable(${TARGET_NAME}  ${CPP_LIST})
target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_LIBS} ${WEBRTC_OBJ})