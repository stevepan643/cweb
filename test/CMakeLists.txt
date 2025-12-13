cmake_minimum_required(VERSION 3.16)
project(cweb_test C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# main 源文件
set(TEST_SOURCES
    src/main.c
)

# ------------------- 引入库 -------------------
# 使用 add_subdirectory 引入 cweb 库
add_subdirectory(../cweb cweb_build)

# ------------------- 构建可执行文件 -------------------
add_executable(cweb_test ${TEST_SOURCES})

# 这里再 link 库
target_link_libraries(cweb_test PRIVATE cweb_lib)

# include 只看到对外头文件
target_include_directories(cweb_test
    PRIVATE ../cweb/include
)
