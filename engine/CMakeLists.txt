cmake_minimum_required(VERSION 3.31)
include(GenerateExportHeader)

project(rats-engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
if(MSVC)
	add_compile_options(/Zc:__cplusplus /permissive- /Zc:preprocessor)
endif()

option(RATS_ENGINE_WINDOW_GLFW "Use GLFW for window managing" OFF)
option(RATS_ENGINE_RENDER_VULKAN "Add support of Vulkan render API" OFF)

find_package(fmt CONFIG REQUIRED)
find_package(EASTL CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
list(APPEND RATS_ENGINE_PUBLIC_LIBS 
	fmt::fmt 
	EASTL
	glm::glm)
list(APPEND RATS_ENGINE_PRIVATE_LIBS)

list(APPEND RATS_ENGINE_PUBLIC_DEFINITIONS
	"RATS_ENGINE_DEBUG=$<IF:$<CONFIG:Debug>,1,0>")
list(APPEND RATS_ENGINE_PRIVATE_DEFINITIONS)
list(APPEND RATS_ENGINE_PUBLIC_PCH
	"include/engine/core.h")
list(APPEND RATS_ENGINE_PUBLIC_HEADERS
	"include/engine/engine.h"
	"include/engine/utils/log.h"
	"include/engine/utils/uuid.h"
	"include/engine/utils/macro/defer.h")
list(APPEND RATS_ENGINE_PRIVATE_HEADERS)
list(APPEND RATS_ENGINE_SRC_FILES
	"src/engine.cpp"
	"src/utils/eastl_operator_new.cpp"
	"src/utils/log.cpp"
	"src/utils/uuid.cpp")

if(${RATS_ENGINE_RENDER_VULKAN})

	find_package(Vulkan REQUIRED)
	list(APPEND RATS_ENGINE_PRIVATE_LIBS Vulkan::Vulkan)

endif()

if(${RATS_ENGINE_WINDOW_GLFW})

	find_package(glfw3 CONFIG REQUIRED)
	list(APPEND RATS_ENGINE_PRIVATE_LIBS glfw)

endif()

add_library(rats-engine SHARED)
target_link_libraries(rats-engine
	PUBLIC ${RATS_ENGINE_PUBLIC_LIBS}
	PRIVATE ${RATS_ENGINE_PRIVATE_LIBS})
target_compile_definitions(rats-engine
	PUBLIC ${RATS_ENGINE_PUBLIC_DEFINITIONS}
	PRIVATE ${RATS_ENGINE_PRIVATE_DEFINITIONS})

set_target_properties(rats-engine PROPERTIES 
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON)
generate_export_header(rats-engine EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/engine/engine_export.h")
target_include_directories(rats-engine
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include"
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")

target_precompile_headers(rats-engine PUBLIC ${RATS_ENGINE_PUBLIC_PCH})
target_sources(rats-engine PUBLIC
	FILE_SET public_headers
		TYPE HEADERS
		BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
		FILES ${RATS_ENGINE_PUBLIC_HEADERS})
target_sources(rats-engine PRIVATE
		FILE_SET private_headers
		TYPE HEADERS
		BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/private"
		FILES ${RATS_ENGINE_PRIVATE_HEADERS})
target_sources(rats-engine PRIVATE ${RATS_ENGINE_SRC_FILES})
