cmake_minimum_required(VERSION 3.11)
project(VideoBat VERSION 0.0.1)

#set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)
#list(APPEND PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/core)
set(PROJECT_LIBS)

#set(CMAKE_CXX_COMPILER "clang-cl")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-Wno-invalid-offsetof \
#-Wenum-compare-conditional \
#-Wno-c++11-narrowing-const-reference \
#-Wno-missing-template-arg-list-after-template-kw \
#-Wno-dangling-assignment-gsl \
#/std:c++17 ${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS "/std:c++17 /EHsc /MT /Zi /Ob0 /Od /RTC1 ${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
find_package(Threads REQUIRED)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MT")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -MT")
# Find webrtc include and library file
set(LIBWEBRTC_PATH "D:/tools/webrtc/libwebrtc/src")

list(APPEND PROJECT_INCLUDE ${LIBWEBRTC_PATH})
list(APPEND PROJECT_INCLUDE "${LIBWEBRTC_PATH}/third_party/abseil-cpp")
list(APPEND PROJECT_INCLUDE "${LIBWEBRTC_PATH}/third_party/jsoncpp/source/include")
list(APPEND PROJECT_INCLUDE "${LIBWEBRTC_PATH}/third_party/libyuv/include")

#find_library(WEBRTC_LIB webrtc PATHS "${LIBWEBRTC_PATH}/out/Release/obj/" REQUIRED)
file(GLOB_RECURSE WEBRTC_LIB "${LIBWEBRTC_PATH}/out/release_h264/obj/*.lib")
file(GLOB_RECURSE WEBRTC_OBJ "D:/tools/webrtc/lib/*.obj")
#find_file(ABSL_FLAG_OBJ flag.obj PATHS ${LIBWEBRTC_BINARY_PATH}/absl/obj/)
#find_file(ABSL_CLF_OBJ commandlineflag.obj PATHS ${LIBWEBRTC_BINARY_PATH}/absl/obj/)
#find_file(ABSL_CLF_INTER_OBJ commandlineflag.obj PATHS ${LIBWEBRTC_BINARY_PATH}/absl/obj/internal/)
#find_file(ABSL_UC_OBJ usage_config.obj PATHS ${LIBWEBRTC_BINARY_PATH}/absl/obj/)
#find_file(ABSL_USAGE_OBJ usage.obj PATHS ${LIBWEBRTC_BINARY_PATH}/absl/obj/)
#find_file(ABSL_MTX_OBJ mutex.obj PATHS ${LIBWEBRTC_BINARY_PATH}/absl/obj/)
#find_file(ABSL_PARSE_OBJ parse.obj PATHS ${LIBWEBRTC_BINARY_PATH}/absl/obj/)
list(APPEND PROJECT_LIBS ${WEBRTC_OBJ} ${WEBRTC_LIB})
#list(APPEND PROJECT_LIBS ${JS_OBJ} ${JSR_OBJ} ${JSV_OBJ} ${JSW_OBJ} ${FT_OBJ})
#list(APPEND WEBRTC_OBJ ${TVC_OBJ} ${VCM_OBJ} ${JS_OBJ} ${JSV_OBJ} ${JSR_OBJ} ${JSW_OBJ})

#message(STATUS "webrtc obj:" ${WEBRTC_OBJ})

set(CMAKE_PREFIX_PATH "D:/tools/SFML-2.6.1/lib/cmake/SFML" ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "D:/tools/SFML-2.6.1/lib" ${CMAKE_PREFIX_PATH})
set(SFML_INCLUDE_DIRS "D:/tools/SFML-2.6.1/include" ${SFML_INCLUDE_DIRS})
set(FFMPEG_INCLUDE_DIR "E:/tool/ffmpeg_sdl/include" ${FFMPEG_INCLUDE_DIR})
set(CMAKE_PREFIX_PATH "E:/tool/ffmpeg_sdl/lib" ${CMAKE_PREFIX_PATH})

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



######################################
#message("============= FFmpeg lib setting  ==============")
#######################################
##make sure ffmpeg/bin is in your PATH to RUN.
#find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h REQUIRED)
#find_library(AVCODEC_LIBRARY avcodec REQUIRED)
#find_library(AVFORMAT_LIBRARY avformat REQUIRED)
#find_library(AVUTIL_LIBRARY avutil REQUIRED)
#find_library(SWSCALE_LIBRARY swscale REQUIRED)
#find_library(SWRESAMPLE_LIBRARY swresample REQUIRED)
#find_library(AVDEVICE_LIBRARY avdevice REQUIRED)
#find_library(AVFILTER_LIBRARY avfilter REQUIRED)
#find_library(POSTPROC_LIBRARY postproc REQUIRED)
#find_library(SDL_LIBRARY SDL REQUIRED)
#find_library(SDL2_LIBRARY SDL2 REQUIRED)
#find_library(SDLMAIN_LIBRARY SDL2main REQUIRED)
#    
#set(FFMPEG_LIBS
#		${AVCODEC_LIBRARY}
#		${AVFORMAT_LIBRARY}
#		${AVUTIL_LIBRARY}
#		${SWSCALE_LIBRARY}
#		${SWRESAMPLE_LIBRARY}
#		${AVDEVICE_LIBRARY}
#		${AVFILTER_LIBRARY}
#		${POSTPROC_LIBRARY}
#		#${SDL_LIBRARY}
#		${SDL2_LIBRARY}
#		${SDLMAIN_LIBRARY}
#)
#
#message(FFMPEG_INCLUDE_DIR       =  ${FFMPEG_INCLUDE_DIR})
#message(FFMPEG_LIBS              =  ${FFMPEG_LIBS})
#
#
#######################################
#message("============= opencv setting  ==============")
#######################################
#find_package(OpenCV REQUIRED)
#message(STATUS "OpenCV library status:")
#message(STATUS "version: ${OpenCV_VERSION}")
#message(STATUS "libraries: ${OpenCV_LIBS}")
#message(STATUS "include path: ${OpenCV_INCLUDE_DIRS}")


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

message(STATUS "link lib:${PROJECT_LIBS}")

#add_definitions(-DWEBRTC_WIN)
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
-DWEBRTC_USE_H264 #设置使用h264
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
#add_subdirectory(test/echoTest)
#add_subdirectory(test/newEchoTest)
#add_subdirectory(test/echoApiTest)
#add_subdirectory(test/simple)
#add_subdirectory(test/baseTest)
add_subdirectory(test/screenCaptureTest)
#add_subdirectory(src)