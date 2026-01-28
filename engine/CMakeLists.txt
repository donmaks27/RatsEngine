cmake_minimum_required(VERSION 3.31)
include(GenerateExportHeader)

project(rats-engine CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
if(MSVC)
	add_compile_options(/Zc:__cplusplus /permissive- /Zc:preprocessor)
endif()

find_package(fmt CONFIG REQUIRED)
list(APPEND RATS_ENGINE_PUBLIC_LIBS fmt::fmt-header-only)

add_library(rats-engine SHARED)

set_target_properties(rats-engine PROPERTIES 
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON)
generate_export_header(rats-engine EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/src/engine_export.h")
target_include_directories(rats-engine
	PRIVATE src
)

target_sources(rats-engine PUBLIC
	FILE_SET CXX_MODULES
		BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"
		FILES
			"src/engine.cppm"
			"src/utils/utils.cppm"
			"src/utils/log.cppm")
target_sources(rats-engine PRIVATE
	"src/engine_export.h"
	"src/engine.cpp"
	
	"src/utils/log.cpp" )

target_link_libraries(rats-engine PUBLIC ${RATS_ENGINE_PUBLIC_LIBS})