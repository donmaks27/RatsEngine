cmake_minimum_required(VERSION 3.31)

project(rats-game CXX)

list(APPEND RATS_ENGINE_GAME_LIBS
    rats-engine-runtime)

list(APPEND RATS_ENGINE_GAME_DEFINITIONS)
list(APPEND RATS_ENGINE_GAME_PCH
    "src/pch.h")
list(APPEND RATS_ENGINE_GAME_HEADERS
    "include/game/core.h"
    "include/game/game_engine.h")
list(APPEND RATS_ENGINE_GAME_SRC_FILES
    "src/main.cpp"
    "src/game_engine.cpp")
list(APPEND RATS_ENGINE_GAME_SRC_FILES_NO_PCH)

add_executable(rats-game ${RATS_ENGINE_GAME_SRC_FILES} ${RATS_ENGINE_GAME_SRC_FILES_NO_PCH})
target_link_libraries(rats-game PRIVATE ${RATS_ENGINE_GAME_LIBS})
target_compile_definitions(rats-game PRIVATE ${RATS_ENGINE_GAME_DEFINITIONS})

target_include_directories(rats-game PRIVATE "include" "${CMAKE_CURRENT_BINARY_DIR}/include")

target_precompile_headers(rats-game PRIVATE ${RATS_ENGINE_GAME_PCH})
target_sources(rats-game PRIVATE
    FILE_SET headers
    TYPE HEADERS
    BASE_DIRS "include"
    FILES ${RATS_ENGINE_GAME_HEADERS})
set_source_files_properties(${RATS_ENGINE_GAME_SRC_FILES_NO_PCH} PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON)