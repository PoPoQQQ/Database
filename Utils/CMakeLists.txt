# create library

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(. DIR_LIB_SRCS)
add_library(utils ${DIR_LIB_SRCS})

# complier options

add_compile_options(-Wall -g)
