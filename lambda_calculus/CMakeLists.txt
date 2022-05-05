add_executable(
    lambda_calculus
    lambda_calculus.cpp
)
target_include_directories(lambda_calculus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lambda_calculus PRIVATE shared_settings)
