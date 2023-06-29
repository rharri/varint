cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(varint)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_DEBUG_FLAGS "-g -ggdb3 -O0 -ggdb -Wall -Wconversion -Werror -Wextra -Wpedantic")
set(CMAKE_C_RELEASE_FLAGS "-Wall -Wconversion -Werror -Wextra -Wpedantic")

set(SOURCE_FILES src/varint.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})