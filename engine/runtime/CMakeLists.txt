cmake_minimum_required(VERSION 3.31)
include(GenerateExportHeader)

project(rats-engine-runtime CXX)

add_library(rats-engine-runtime STATIC dummy.cpp)
target_link_libraries(rats-engine-runtime PUBLIC rats-engine)