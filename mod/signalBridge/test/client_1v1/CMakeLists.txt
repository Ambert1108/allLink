set(TARGET_NAME "client_1v1")

message(PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR})

add_executable(${TARGET_NAME}
        main.cpp
        WSListener.cpp
        conductor.cpp
        defaults.cpp
)

target_include_directories(${TARGET_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/signalBridge_old/core
        ${PROJECT_INCLUDE}
)

message(STATUS "project include:${PROJECT_SOURCE_DIR}")

target_link_libraries(${TARGET_NAME}
        PRIVATE
        ${PROJECT_LIBS}
        ${WEBRTC_OBJ}
)