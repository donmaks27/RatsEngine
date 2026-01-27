cmake_minimum_required(VERSION 3.31)
include(GenerateExportHeader)

project(engine CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
	add_compile_options(/Zc:__cplusplus /permissive- /Zc:preprocessor)
endif()

find_package(fmt CONFIG REQUIRED)
list(APPEND ENGINE_PUBLIC_LIBS fmt::fmt-header-only)

add_library(engine SHARED)
generate_export_header(engine EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/src/private/engine_export.h")
target_sources(engine
	PUBLIC
		FILE_SET CXX_MODULES
			BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/public"
			FILES
				"src/public/engine.cppm"
	PRIVATE
		"src/private/engine.cpp"
)
target_include_directories(engine
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/private"
)
target_link_libraries(engine PUBLIC ${ENGINE_PUBLIC_LIBS})