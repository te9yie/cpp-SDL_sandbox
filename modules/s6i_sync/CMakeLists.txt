cmake_minimum_required(VERSION 3.19)
project(s6i_sync)


# s6i_sync
add_library(s6i_sync INTERFACE)
target_include_directories(s6i_sync INTERFACE include)
target_link_libraries(s6i_sync INTERFACE
    cpp_base
    s6i_result
    SDL2::SDL2-static
)


# ユニットテスト
if(SDL_SANDBOX_ENABLE_TESTS)
    add_executable(${PROJECT_NAME}_tests
        tests/mutex_test.cpp
    )
    target_precompile_headers(${PROJECT_NAME}_tests PRIVATE tests/pch.h)
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()
