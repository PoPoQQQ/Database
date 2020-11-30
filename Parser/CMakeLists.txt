# bison and flex 

find_package(BISON)
find_package(FLEX)

FLEX_TARGET(lexer lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp)

BISON_TARGET(parser yacc.y ${CMAKE_CURRENT_BINARY_DIR}/yacc.tab.cpp
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/yacc.tab.h
            )

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

# create library

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(myparser ${BISON_parser_OUTPUTS} ${FLEX_lexer_OUTPUTS})

# complier options

add_compile_options(-Wall -g)
