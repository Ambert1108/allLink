set(TARGET_NAME "signalBridge.${PROJECT_VERSION}")

message(PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR})

add_executable(${TARGET_NAME}
#        main.cpp
#        reConnect.cpp
#        loseNet.cpp
#        defaults.cpp
#        conductor.cpp
#        ../include/signalBridge_old/connectEngine.cpp
#        ../include/signalBridge_old/core/EngineListener.cpp

        ../include/signalBridge/engineListener.cpp
        ../include/signalBridge/rtcConnectEngine.cpp
        ../include/signalBridge/defaults.cpp
        ../include/signalBridge/AudioEngine/rtcAudioEngine.cpp
        ../include/signalBridge/VideoEngine/rtcVideoEngine.cpp
        ../include/signalBridge/VideoEngine/screenCapturer.cpp

        main_2.cpp
#        codecTypeTest.cpp
#        logoutTest.cpp
#        joinMeethingTest.cpp
)

target_include_directories(${TARGET_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include/signalBridge
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_INCLUDE}
)

message(STATUS "project include:${PROJECT_SOURCE_DIR}/include")

target_link_libraries(${TARGET_NAME}
        PRIVATE
        ${PROJECT_LIBS}
        ${WEBRTC_OBJ}
)