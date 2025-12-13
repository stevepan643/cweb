cmake_minimum_required(VERSION 3.16)
project(c_web C)

set(CMAKE_C_STANDARD 11)

include_directories(./include)

add_library(c_web SHARED
    src/http.c
    src/http.h
)

if(WIN32)
    target_sources(c_web PRIVATE
        src/platform_win32.c
    )
    target_compile_definitions(c_web PRIVATE PLATFORM_WINDOWS)
    target_link_libraries(c_web ws2_32)
else()
endif()