set(TARGET_NAME "echoTest")

message(PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR})

add_executable("echoTest" 
        main.cpp
       
        )

target_include_directories("echoTest"
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_INCLUDE}
)


target_link_libraries("echoTest"
        PRIVATE
        ${PROJECT_LIBS}
)
