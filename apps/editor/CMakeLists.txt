cmake_minimum_required(VERSION 3.31)

project(rats-editor CXX)

list(APPEND RATS_ENGINE_EDITOR_LIBS
    rats-engine-runtime)

list(APPEND RATS_ENGINE_EDITOR_DEFINITIONS)
list(APPEND RATS_ENGINE_EDITOR_PCH
    "src/pch.h")
list(APPEND RATS_ENGINE_EDITOR_HEADERS
    "include/editor/core.h"
    "include/editor/editor_engine.h")
list(APPEND RATS_ENGINE_EDITOR_SRC_FILES
    "src/main.cpp"
    "src/editor_engine.cpp")
list(APPEND RATS_ENGINE_EDITOR_SRC_FILES_NO_PCH)

add_executable(rats-editor ${RATS_ENGINE_EDITOR_SRC_FILES} ${RATS_ENGINE_EDITOR_SRC_FILES_NO_PCH})
target_link_libraries(rats-editor PRIVATE ${RATS_ENGINE_EDITOR_LIBS})
target_compile_definitions(rats-editor PRIVATE ${RATS_ENGINE_EDITOR_DEFINITIONS})

target_include_directories(rats-editor PRIVATE "include" "${CMAKE_CURRENT_BINARY_DIR}/include")

target_precompile_headers(rats-editor PRIVATE ${RATS_ENGINE_EDITOR_PCH})
target_sources(rats-editor PRIVATE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "include"
    FILES ${RATS_ENGINE_EDITOR_HEADERS})
set_source_files_properties(${RATS_ENGINE_EDITOR_SRC_FILES_NO_PCH} PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON)