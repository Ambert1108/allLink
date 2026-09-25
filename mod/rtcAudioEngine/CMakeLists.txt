cmake_minimum_required (VERSION 3.11)

project ("rtcAudioEngine" VERSION 0.0.0)

include(./cmake/.LOCAL_SETTING.cmake)

include(./cmake/checkBuildType.cmake)

# --------------------------------------------------------------------------
# specify the C++ standard
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_COMPILER "clang-cl")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
find_package(Threads REQUIRED)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MT")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -MT")

######################################
message("============= FFmpeg lib setting  ==============")
######################################
#make sure ffmpeg/bin is in your PATH to RUN.
find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h REQUIRED)
find_library(AVCODEC_LIBRARY avcodec REQUIRED)
find_library(AVFORMAT_LIBRARY avformat REQUIRED)
find_library(AVUTIL_LIBRARY avutil REQUIRED)
find_library(SWSCALE_LIBRARY swscale REQUIRED)
find_library(SWRESAMPLE_LIBRARY swresample REQUIRED)
find_library(AVDEVICE_LIBRARY avdevice REQUIRED)
find_library(AVFILTER_LIBRARY avfilter REQUIRED)
find_library(POSTPROC_LIBRARY postproc REQUIRED)
find_library(SDL_LIBRARY SDL REQUIRED)
find_library(SDL2_LIBRARY SDL2 REQUIRED)
find_library(SDLMAIN_LIBRARY SDL2main REQUIRED)
    
set(FFMPEG_LIBS
		${AVCODEC_LIBRARY}
		${AVFORMAT_LIBRARY}
		${AVUTIL_LIBRARY}
		${SWSCALE_LIBRARY}
		${SWRESAMPLE_LIBRARY}
		${AVDEVICE_LIBRARY}
		${AVFILTER_LIBRARY}
		${POSTPROC_LIBRARY}
)

message(FFMPEG_INCLUDE_DIR       =  ${FFMPEG_INCLUDE_DIR})
message(FFMPEG_LIBS              =  ${FFMPEG_LIBS})


######################################
message("============= opencv setting  ==============")
######################################
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "version: ${OpenCV_VERSION}")
message(STATUS "libraries: ${OpenCV_LIBS}")
message(STATUS "include path: ${OpenCV_INCLUDE_DIRS}")


message("============= webrtc lib setting  ==============")
list(APPEND WEBRTC_INCLUDE ${LIBWEBRTC_INCLUDE_PATH})
list(APPEND WEBRTC_INCLUDE "${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp")
list(APPEND WEBRTC_INCLUDE "${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp/absl")
list(APPEND WEBRTC_INCLUDE "${LIBWEBRTC_INCLUDE_PATH}/third_party/jsoncpp/source/include")
list(APPEND WEBRTC_INCLUDE "${LIBWEBRTC_INCLUDE_PATH}/third_party/libyuv/include")


file(GLOB_RECURSE WEBRTC_LIB "${LIBWEBRTC_INCLUDE_PATH}/out/Release/obj/*.lib")
file(GLOB_RECURSE WEBRTC_OBJ "${LIBWEBRTC_INCLUDE_PATH}/obj/*.obj")
list(APPEND WEBRTC_LIBS ${WEBRTC_OBJ} ${WEBRTC_LIB})
message(STATUS "WEBRTC_LIB " ${WEBRTC_LIB})
message(STATUS "obj:" ${WEBRTC_OBJ})

list(APPEND WEBRTC_LIBS 
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
message(STATUS "webrtc include:" ${WEBRTC_INCLUDE})
message(STATUS "webrtc lib:" ${WEBRTC_LIBS})

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

add_subdirectory(test/echoTest)
#add_subdirectory(test/sdpTest)
#add_subdirectory(test/audioTest)