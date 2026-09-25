set(TARGET_NAME "WebrtcTest")

message(PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR})

add_executable("WebrtcTest" 
        main.cpp
        conductor.cpp
        main_wnd.cpp 
        peerconnection.cpp
        defaults.cpp
        )

target_include_directories("WebrtcTest"
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_INCLUDE}
)


target_link_libraries("WebrtcTest"
        PRIVATE
        ${PROJECT_LIBS}
)
