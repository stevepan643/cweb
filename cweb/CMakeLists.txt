cmake_minimum_required(VERSION 3.16)
project(cweb C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------- 源文件 -------------------
set(HTTP_SOURCES
    src/http/http_parser.c
    src/http/http_request.c
    src/http/http_response.c
    src/http/http_server.c
)

set(LOG_SOURCES
    src/log/log_core.c
    src/log/log_rotate.c
)

set(FILE_SOURCES
    src/utils/file/file.c
)

if(WIN32)
    set(PLATFORM_SOURCES
        src/utils/platform/platform_win32.c
    )
    add_definitions(-DPLATFORM_WINDOWS)
    set(PLATFORM_LIBS ws2_32)
else()
    set(PLATFORM_SOURCES
        src/utils/platform/platform_posix.c
    )
    set(PLATFORM_LIBS pthread)
endif()

# ------------------- 构建库 -------------------
add_library(cweb_lib STATIC
    ${HTTP_SOURCES}
    ${LOG_SOURCES}
    ${PLATFORM_SOURCES}
    ${FILE_SOURCES}
)

# 对外头文件
target_include_directories(cweb_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接平台库
target_link_libraries(cweb_lib ${PLATFORM_LIBS})

# 导出库供其他项目使用
export(TARGETS cweb_lib FILE cweb_lib.cmake)
