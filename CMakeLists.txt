cmake_minimum_required(VERSION 3.14)
project(ngram)

set(CMAKE_CXX_STANDARD 11)

set(ENABLE_FAR TRUE)
add_subdirectory(openfst)

file(GLOB_RECURSE ngram_lib_src src/lib/*.cc)
file(GLOB_RECURSE ngram_bin_src src/bin/*.cc)
file(GLOB_RECURSE ngram_bin_src_main src/bin/*main.cc)
list(REMOVE_ITEM ngram_bin_src ${ngram_bin_src_main})

add_library(NGRAM_LIB ${ngram_lib_src} ${ngram_bin_src_main})
target_link_libraries(NGRAM_LIB OPENFST_LIB)
target_include_directories(NGRAM_LIB PUBLIC src/include)

foreach(item ${ngram_bin_src})
    get_filename_component(binary ${item} NAME_WE)
    add_executable(${binary} ${item})
    target_link_libraries(${binary} NGRAM_LIB)
endforeach(item ${ngram_bin_src})