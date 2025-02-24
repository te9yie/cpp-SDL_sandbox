cmake_minimum_required(VERSION 3.19)
project(SDL_sandbox)


# オプション設定
option(SDL_SANDBOX_ENABLE_EXAMPLES "Enable examples" OFF)
option(SDL_SANDBOX_ENABLE_TESTS "Enable unit tests" OFF)


# cpp_base （基本設定）
add_library(cpp_base INTERFACE)

## C++17
target_compile_features(cpp_base INTERFACE cxx_std_17)

## 警告レベル最大、例外無効、RTTI無効
target_compile_options(cpp_base INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /W4 /Wall /EHa- /GR->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -fno-exceptions -fno-rtti>
)
target_compile_definitions(cpp_base INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:_HAS_EXCEPTIONS=0>
)


include(FetchContent)


# SDL2
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.x
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2)


# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.91.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)


# examples
if(SDL_SANDBOX_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()


# tests
if(SDL_SANDBOX_ENABLE_TESTS)
    # GoogleTest
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.16.x
        GIT_SHALLOW TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
endif()
