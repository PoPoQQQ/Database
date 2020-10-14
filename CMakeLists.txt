# CMake 最低版本号要求

cmake_minimum_required (VERSION 3.10)

# 项目信息

project (BDDB)

# 添加子目录

# add_subdirectory(bufmanager)
# add_subdirectory(fileio)
# add_subdirectory(utils)

# 指定生成目标

add_executable(main main.cpp)

# 链接
