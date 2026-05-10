cmake_minimum_required(VERSION 3.31)
include(GenerateExportHeader)

project(rats-engine-runtime CXX)

option(RATS_ENGINE_RUNTIME_GLFW_ENABLE "Use GLFW for window managing" OFF)
option(RATS_ENGINE_RUNTIME_VULKAN_ENABLE "Add support of Vulkan render API" OFF)
option(RATS_ENGINE_RUNTIME_VULKAN_ENABLE_VALIDATION_LAYERS "Enable validation layers for Vulkan render API" OFF)

cmake_option_to_bool(RATS_ENGINE_RUNTIME_GLFW_ENABLE)
cmake_option_to_bool(RATS_ENGINE_RUNTIME_VULKAN_ENABLE)
cmake_option_to_bool(RATS_ENGINE_RUNTIME_VULKAN_ENABLE_VALIDATION_LAYERS)

list(APPEND RATS_ENGINE_RUNTIME_PUBLIC_LIBS
    rats-engine)
list(APPEND RATS_ENGINE_RUNTIME_PRIVATE_LIBS)

list(APPEND RATS_ENGINE_RUNTIME_PUBLIC_DEFINITIONS)
list(APPEND RATS_ENGINE_RUNTIME_PRIVATE_DEFINITIONS)
list(APPEND RATS_ENGINE_RUNTIME_PRIVATE_PCH
    "src/pch.h")
list(APPEND RATS_ENGINE_RUNTIME_PUBLIC_HEADERS
    "include/engine/runtime/core.h"
    "include/engine/runtime/runtime_engine.h")
list(APPEND RATS_ENGINE_RUNTIME_PRIVATE_HEADERS)
list(APPEND RATS_ENGINE_RUNTIME_SRC_FILES
    "src/runtime_engine.cpp")
list(APPEND RATS_ENGINE_RUNTIME_SRC_FILES_NO_PCH)

add_library(rats-engine-runtime STATIC)
target_link_libraries(rats-engine-runtime
    PUBLIC ${RATS_ENGINE_RUNTIME_PUBLIC_LIBS}
    PRIVATE ${RATS_ENGINE_RUNTIME_PRIVATE_LIBS})
target_compile_definitions(rats-engine-runtime
    PUBLIC ${RATS_ENGINE_RUNTIME_PUBLIC_DEFINITIONS}
    PRIVATE ${RATS_ENGINE_RUNTIME_PRIVATE_DEFINITIONS})

set_target_properties(rats-engine-runtime PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)
generate_export_header(rats-engine-runtime EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/engine/runtime/engine_export.h")
configure_file(
    "cmake/configs/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/engine/runtime/config.h"
    @ONLY)
configure_file(
    "cmake/configs/config_macro.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/engine/runtime/config_macro.h"
    @ONLY)
target_include_directories(rats-engine-runtime
    PUBLIC "include" "${CMAKE_CURRENT_BINARY_DIR}/include"
    PRIVATE "private" "${CMAKE_CURRENT_BINARY_DIR}/private")

target_precompile_headers(rats-engine-runtime PRIVATE ${RATS_ENGINE_RUNTIME_PRIVATE_PCH})
target_sources(rats-engine-runtime PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS "include"
    FILES ${RATS_ENGINE_RUNTIME_PUBLIC_HEADERS})
target_sources(rats-engine-runtime PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    BASE_DIRS "private"
    FILES ${RATS_ENGINE_RUNTIME_PRIVATE_HEADERS})
target_sources(rats-engine-runtime PRIVATE ${RATS_ENGINE_RUNTIME_SRC_FILES} ${RATS_ENGINE_RUNTIME_SRC_FILES_NO_PCH})
set_source_files_properties(${RATS_ENGINE_RUNTIME_SRC_FILES_NO_PCH} PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON)