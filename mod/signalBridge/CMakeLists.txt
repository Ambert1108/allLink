cmake_minimum_required(VERSION 3.11)
project(signalBridge VERSION 0.0.2)

#set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)
#list(APPEND PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_INCLUDE)
set(PROJECT_LIBS)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_COMPILER "clang-cl")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MT")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -MT")

set(CMAKE_PREFIX_PATH "C:/Program Files/oatpp/lib/cmake/oatpp-1.3.0" ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "C:/Program Files/oatpp-websocket/lib/cmake/oatpp-websocket-1.3.0" ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "C:/Program Files/oatpp/lib" ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "C:/Program Files/oatpp-websocket/lib" ${CMAKE_PREFIX_PATH})

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

#set(LIBWEBRTC_PATH "D:/EB/webrtc_1022/checkout/src")
set(LIBWEBRTC_PATH "D:/EB/lib/webrtc/src")
list(APPEND PROJECT_INCLUDE ${LIBWEBRTC_PATH})
list(APPEND PROJECT_INCLUDE "${LIBWEBRTC_PATH}/third_party/abseil-cpp")
list(APPEND PROJECT_INCLUDE "${LIBWEBRTC_PATH}/third_party/jsoncpp/source/include")
list(APPEND PROJECT_INCLUDE "${LIBWEBRTC_PATH}/third_party/libyuv/include")


#file(GLOB_RECURSE WEBRTC_LIB "${LIBWEBRTC_PATH}/out_M129/release_x64/obj/*.lib")
file(GLOB_RECURSE WEBRTC_LIB "${LIBWEBRTC_PATH}/out/Release_264/obj/*.lib")
file(GLOB_RECURSE WEBRTC_OBJ "${LIBWEBRTC_PATH}/obj/*.obj")
list(APPEND PROJECT_LIBS ${WEBRTC_OBJ} ${WEBRTC_LIB})
message(STATUS "webrtc obj:" ${WEBRTC_OBJ})

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
        strmiids.lib
        d3d11.lib
        dwmapi.lib
        dxgi.lib
        Shcore.lib
)

message(STATUS "link lib:${PROJECT_LIBS}")

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
add_subdirectory(test)
