cmake_minimum_required(VERSION 3.10)
project(CubeSolver VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Derleyici optimizasyonları
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O3)
endif()

find_package(Threads REQUIRED)

add_executable(cube_solver src/main.cpp)
target_link_libraries(cube_solver PRIVATE Threads::Threads)