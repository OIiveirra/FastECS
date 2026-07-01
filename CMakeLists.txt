cmake_minimum_required(VERSION 3.15)
project(FastECS CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX /utf-8 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

include_directories(include)

add_executable(FastECS_Test src/main.cpp src/MemoryPool.cpp)
