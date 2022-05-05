cmake_minimum_required(VERSION 3.13) # 3.13 is required for target_link_options
project(LambdaCalculus CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(cmake/shared_settings.cmake)
add_subdirectory(lambda_calculus)
