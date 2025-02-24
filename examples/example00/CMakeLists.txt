cmake_minimum_required(VERSION 3.19)
project(example00)


# example00
add_executable(${PROJECT_NAME} WIN32 main.cpp)
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
target_link_libraries(${PROJECT_NAME} PRIVATE
    cpp_base
    SDL2::SDL2-static
    SDL2::SDL2main
)
