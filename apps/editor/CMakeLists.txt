cmake_minimum_required(VERSION 3.31)

project(rats-engine-editor CXX)

add_executable(rats-editor "main.cpp")
target_link_libraries(rats-editor PRIVATE rats-engine-runtime)