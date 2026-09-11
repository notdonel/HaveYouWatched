cmake_minimum_required(VERSION 3.16)
project(media-tracker C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    cjson
    URL https://github.com/DaveGamble/cJSON/archive/refs/tags/v1.7.18.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_UNINSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cjson)

add_executable(tracker
    src/main.c
    src/cli.c
    src/library.c
    src/storage.c
    src/model.c
    src/util.c
)

target_include_directories(tracker PRIVATE src)
target_link_libraries(tracker PRIVATE cjson)

if(MSVC)
    target_compile_options(tracker PRIVATE /W4)
else()
    target_compile_options(tracker PRIVATE -Wall -Wextra -Wpedantic)
endif()
