cmake_minimum_required(VERSION 3.31)

project(rats-engine-game CXX)

add_executable(rats-game "main.cpp")
target_link_libraries(rats-game PRIVATE rats-engine-runtime)