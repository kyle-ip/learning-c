cmake_minimum_required(VERSION 3.10)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId} C)

set(CMAKE_C_STANDARD 90)

# 添加搜索路径（#include <...h>）
include_directories("include")

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
# 遍历工程下所有文件
foreach(file ${files})
    get_filename_component(name ${file} NAME)

    # 添加可执行文件
    add_executable(${name} ${file} src/factorial.c)

    # 定义 DEBUG
    target_compile_definitions(${name} PUBLIC DEBUG)
endforeach()