cmake_minimum_required(VERSION 3.19)
project(s6i_result)


# s6i_result
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE cpp_base)


# ユニットテスト
if(SDL_SANDBOX_ENABLE_TESTS)
    add_executable(${PROJECT_NAME}_tests
        tests/result_test.cpp
    )
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()
