# CMake 最低版本号要求

cmake_minimum_required (VERSION 3.10)

# 项目信息

project (parser_test)

# bison and flex 

find_package(BISON)
find_package(FLEX)

FLEX_TARGET(lexer lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp)

BISON_TARGET(parser yacc.y ${CMAKE_CURRENT_BINARY_DIR}/yacc.tab.cpp
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/yacc.tab.h
            )

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

# 

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(myparser ${BISON_parser_OUTPUTS} ${FLEX_lexer_OUTPUTS})


# 指定生成目标

add_executable(ttest main.cpp)

target_link_libraries(ttest myparser)

find_library(LEX_LIB l)
target_link_libraries(ttest ${LEX_LIB})

# 链接

# 自定义指令