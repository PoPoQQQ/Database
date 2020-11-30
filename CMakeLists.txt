# CMake 最低版本号要求

cmake_minimum_required (VERSION 3.10)

# 项目信息

project (BDDB)

# 添加子目录

# add_subdirectory(Bufmanager)
# add_subdirectory(FileIO)
# add_subdirectory(Utils)
add_subdirectory(Parser)

# bison and flex 

find_package(BISON)
find_package(FLEX)

FLEX_TARGET(lexer lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

BISON_TARGET(parser yacc.h ${CMAKE_CURRENT_BINARY_DIR}/yacc.tab.c
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/yacc.tab.h
            )

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

# 

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set (MAIN_SRC ${BISON_ttest_parser_OUTPUTS}
              ${FLEX_ttest_lexer_OUTPUTS}
    )


# 指定生成目标

# add_executable(main main.cpp)
add_executable(main ${MAIN_SRC})

find_library(LEX_LIB l)
target_link_libraries(ttest ${LEX_LIB})

# 链接

# 自定义指令

## 删除数据库

add_custom_target(
    clean_db
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_BINARY_DIR}/Database"
)
