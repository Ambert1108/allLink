cmake_minimum_required(VERSION 3.11)
project(AllLink VERSION 0.3.1)
set(ObjType console CACHE STRING "build object type")
set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/inc)
list(APPEND PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/core)
list(APPEND PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/core/engine)
set(PROJECT_LIBS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IS_RELEASE on)
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
	set(IS_RELEASE off)
	if (MSVC)
		add_compile_options("/utf-8")
		add_compile_options("/EHsc")  # deal with MSVC Warning C4530
		add_compile_options("/Od")
		add_compile_options(/w44251 /w44275)
	endif (MSVC)
endif()

find_package(Threads REQUIRED)
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MT")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -MT")
	set(CMAKE_PREFIX_PATH "D:/tools/oatpp-1.3.0/build/Release/lib/cmake/oatpp-1.3.0" ${CMAKE_PREFIX_PATH})
	set(CMAKE_PREFIX_PATH "D:/tools/oatpp-websocket-1.3.0/build/Release/lib/cmake/oatpp-websocket-1.3.0" ${CMAKE_PREFIX_PATH})
	set(CMAKE_PREFIX_PATH "D:/tools/oatpp-1.3.0/build/Release/lib" ${CMAKE_PREFIX_PATH})
	set(CMAKE_PREFIX_PATH "D:/tools/oatpp-websocket-1.3.0/build/Release/lib" ${CMAKE_PREFIX_PATH})
endif()
set(CMAKE_PREFIX_PATH "D:/tools/SFML-2.6.1/lib/cmake/SFML" ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "D:/tools/SFML-2.6.1/lib" ${CMAKE_PREFIX_PATH})
set(SFML_INCLUDE_DIRS "D:/tools/SFML-2.6.1/include" ${SFML_INCLUDE_DIRS})

#find SFML include and library file
message("====== FIND SFML LIBRARY AND INCLUDE ======")
#set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2 COMPONENTS graphics window system audio network REQUIRED)
set(SFML_DIR "D:/tools/SFML-2.6.1/lib/cmake/SFML") 
list(APPEND PROJECT_INCLUDE ${SFML_INCLUDE_DIRS})
find_library(GRAPHIC_LIBRARY sfml-graphics REQUIRED)
list(APPEND PROJECT_LIBS sfml-graphics)
find_library(WINDOW_LIBRARY sfml-window REQUIRED)
list(APPEND PROJECT_LIBS sfml-window)
find_library(SYSTEM_LIBRARY sfml-system REQUIRED)
list(APPEND PROJECT_LIBS sfml-system)
list(APPEND PROJECT_LIBS sfml-audio)
list(APPEND PROJECT_LIBS sfml-network)
message(STATUS "version: ${SFML_VERSION} ${BUILD_TYPE_NAME}")
message(STATUS "sfml dir:${SFML_DIR}")
message(STATUS "graphic lib:${GRAPHIC_LIBRARY}")
message(STATUS "window lib:${WINDOW_LIBRARY}")
message(STATUS "system lib:${SYSTEM_LIBRARY}")

#find OpenCV include and library file
message("====== FIND OPENCV LIBRARY AND INCLUDE ======")
set(CMAKE_PREFIX_PATH "D:/tools/opencv-4.5.4/build/install/x64/vc16/bin" ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "D:/tools/opencv-4.5.4/build/install/x64/vc16/lib" ${CMAKE_PREFIX_PATH})
find_package(OpenCV REQUIRED)
list(APPEND PROJECT_INCLUDE ${OpenCV_INCLUDE_DIRS})
list(APPEND PROJECT_LIBS ${OpenCV_LIBS})
message(STATUS "version: ${OpenCV_VERSION} ${BUILD_TYPE_NAME}")
message(STATUS "opencv include:${OpenCV_INCLUDE_DIRS}")
message(STATUS "opencv lib:${OpenCV_LIBS}")

#find WebRTC include and library file
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
	message("====== FIND WEBRTC LIBRARY AND INCLUDE ======")
	set(LIBWEBRTC_PATH "D:/tools/webrtc/libwebrtc/src")
	list(APPEND WEBRTC_INCLUDE ${LIBWEBRTC_PATH})
	list(APPEND WEBRTC_INCLUDE "${LIBWEBRTC_PATH}/third_party/abseil-cpp")
	list(APPEND WEBRTC_INCLUDE "${LIBWEBRTC_PATH}/third_party/jsoncpp/source/include")
	list(APPEND WEBRTC_INCLUDE "${LIBWEBRTC_PATH}/third_party/libyuv/include")

	file(GLOB_RECURSE WEBRTC_LIB "${LIBWEBRTC_PATH}/out/release_h264/obj/*.lib")
	file(GLOB_RECURSE WEBRTC_OBJ "D:/tools/webrtc/lib/*.obj")

	message(STATUS "webrtc include:${WEBRTC_INCLUDE}")
	message(STATUS "webrtc lib:${WEBRTC_LIB}")
	message(STATUS "webrtc obj:${WEBRTC_OBJ}")

	list(APPEND PROJECT_INCLUDE ${WEBRTC_INCLUDE})
	list(APPEND PROJECT_LIBS ${WEBRTC_OBJ} ${WEBRTC_LIB})

	#find OATPP include and library file
	message("====== FIND OATPP LIBRARY AND INCLUDE ======")
	find_package(oatpp 1.3.0 REQUIRED)
	list(APPEND PROJECT_INCLUDE ${oatpp_INCLUDE_DIRS})
	list(APPEND PROJECT_LIBS oatpp::oatpp)
	message(STATUS "oatpp include: ${oatpp_INCLUDE_DIRS}")
	message(STATUS "oatpp lib: ${oatpp_LIBRARIES}")

	#find OATPP-WebSocket include and library file
	message("====== FIND OATPP_WS LIBRARY AND INCLUDE ======")
	find_package(oatpp-websocket 1.3.0  REQUIRED)
	list(APPEND PROJECT_INCLUDE ${oatpp-websocket_INCLUDE_DIRS})
	list(APPEND PROJECT_LIBS oatpp::oatpp-websocket)
	message(STATUS "oatpp include: ${oatpp-websocket_INCLUDE_DIRS}")
	message(STATUS "oatpp lib: ${oatpp-websocket_LIBRARIES}")
endif()

list(APPEND PROJECT_LIBS 
advapi32.lib 
comdlg32.lib 
dbghelp.lib 
dnsapi.lib 
gdi32.lib 
msimg32.lib 
odbc32.lib 
odbccp32.lib 
oleaut32.lib 
shell32.lib 
shlwapi.lib 
user32.lib 
usp10.lib 
uuid.lib 
version.lib 
wininet.lib 
winmm.lib 
winspool.lib 
ws2_32.lib 
delayimp.lib 
kernel32.lib 
ole32.lib 
crypt32.lib 
iphlpapi.lib 
secur32.lib 
dmoguids.lib 
wmcodecdspuuid.lib 
amstrmid.lib 
msdmo.lib 
strmiids.lib)

add_definitions(
-DUSE_AURA=1 
-D_HAS_NODISCARD 
-D_CRT_NONSTDC_NO_WARNINGS 
-D_WINSOCK_DEPRECATED_NO_WARNINGS 
"-DCR_CLANG_REVISION=\"llvmorg-20-init-3847-g69c43468-28\"" 
-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE 
-D_HAS_EXCEPTIONS=0 
-D__STD_C 
-D_CRT_RAND_S 
-D_CRT_SECURE_NO_DEPRECATE 
-D_SCL_SECURE_NO_DEPRECATE 
-D_ATL_NO_OPENGL 
-D_WINDOWS 
-DCERT_CHAIN_PARA_HAS_EXTRA_FIELDS 
-DPSAPI_VERSION=2 
-DWIN32 
-D_SECURE_ATL 
-DWINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP 
-DWIN32_LEAN_AND_MEAN 
-DNOMINMAX 
-D_UNICODE 
-DUNICODE 
-DNTDDI_VERSION=NTDDI_WIN10_NI 
-D_WIN32_WINNT=0x0A00 
-DWINVER=0x0A00 
-DNDEBUG 
-DNVALGRIND 
-DDYNAMIC_ANNOTATIONS_ENABLED=0 
-DWEBRTC_ENABLE_PROTOBUF=1 
-DWEBRTC_STRICT_FIELD_TRIALS=0 
-DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE 
-DWEBRTC_USE_H264
-DRTC_ENABLE_VP9 
-DRTC_DAV1D_IN_INTERNAL_DECODER_FACTORY 
-DWEBRTC_HAVE_SCTP 
-DWEBRTC_ENABLE_AVX2 
-DRTC_ENABLE_WIN_WGC 
-DWEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=0 
-DWEBRTC_WIN 
-DABSL_ALLOCATOR_NOTHROW=1 
-D_ENABLE_EXTENDED_ALIGNED_STORAGE 
-DABSL_FLAGS_STRIP_NAMES=0 
-DCHROMIUM -DLIBYUV_DISABLE_NEON 
-DLIBYUV_DISABLE_SVE 
-DLIBYUV_DISABLE_SME 
-DLIBYUV_DISABLE_LSX 
-DLIBYUV_DISABLE_LASX  
-DHAVE_WEBRTC_VIDEO
)

# Add your source files here
add_subdirectory(src)
#add_subdirectory(test/localCapture)
#add_subdirectory(test/nosip)
add_subdirectory(test/testWidget)
#add_subdirectory(test/testStreamWnd)
#add_subdirectory(test/testCircleAngle)
#add_subdirectory(test/testSeekBar)
#add_subdirectory(test/customWindows)
#add_subdirectory(test/win32Test)
#add_subdirectory(test/outscreenRender)
#add_subdirectory(test/game1)