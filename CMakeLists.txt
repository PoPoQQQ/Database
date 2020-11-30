# CMake 最低版本号要求

cmake_minimum_required (VERSION 3.10)

# 项目信息

project (BDDB)

# 添加子目录

add_subdirectory(Parser)
add_subdirectory(Record)
add_subdirectory(Utils)

# 指定生成目标

add_executable(main main.cpp)

# 编译器选项

add_compile_options(-Wall -g)

# 链接
find_library(LEX_LIB l)
target_link_libraries(main ${LEX_LIB})

target_link_libraries(main myparser)
target_link_libraries(main record)
target_link_libraries(main utils)

# 自定义指令

## 删除数据库

add_custom_target(
    clean_db
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_BINARY_DIR}/Database"
)
