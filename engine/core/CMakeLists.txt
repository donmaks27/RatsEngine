cmake_minimum_required(VERSION 3.31)
include(GenerateExportHeader)

project(rats-engine CXX)

option(RATS_ENGINE_GLFW_ENABLE "Use GLFW for window managing" OFF)
option(RATS_ENGINE_VULKAN_ENABLE "Add support of Vulkan render API" OFF)
option(RATS_ENGINE_VULKAN_ENABLE_VALIDATION_LAYERS "Enable validation layers for Vulkan render API" OFF)

cmake_option_to_bool(RATS_ENGINE_GLFW_ENABLE)
cmake_option_to_bool(RATS_ENGINE_VULKAN_ENABLE)
cmake_option_to_bool(RATS_ENGINE_VULKAN_ENABLE_VALIDATION_LAYERS)

find_package(fmt CONFIG REQUIRED)
find_package(EASTL CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
list(APPEND RATS_ENGINE_PUBLIC_LIBS 
	fmt::fmt 
	EASTL
	glm::glm)
list(APPEND RATS_ENGINE_PRIVATE_LIBS)

list(APPEND RATS_ENGINE_PUBLIC_DEFINITIONS)
list(APPEND RATS_ENGINE_PRIVATE_DEFINITIONS)
list(APPEND RATS_ENGINE_PRIVATE_PCH
	"src/pch.h")
list(APPEND RATS_ENGINE_PUBLIC_HEADERS
	"include/engine/core.h"
	"include/engine/eastl_config.h"
	"include/engine/core_engine.h"
	"include/engine/events.h"
	"include/engine/engine_event_listener.h"
	"include/engine/service.h"
	"include/engine/render/render_api.h"
	"include/engine/render/render_service.h"
	"include/engine/render/surface_id.h"
	"include/engine/render/surface_service.h"
	"include/engine/utils/log.h"
	"include/engine/utils/uuid.h"
	"include/engine/utils/id.h"
	"include/engine/utils/type_storage.h"
	"include/engine/utils/macro/defer.h")
list(APPEND RATS_ENGINE_PRIVATE_HEADERS
	"private/engine/render/window_service.h")
list(APPEND RATS_ENGINE_SRC_FILES
	"src/core_engine.cpp"
	"src/render/render_service.cpp"
	"src/render/surface_service.cpp"
	"src/render/window_service.cpp")
list(APPEND RATS_ENGINE_SRC_FILES_NO_PCH
	"src/utils/eastl_operator_new.cpp"
	"src/utils/log.cpp"
	"src/utils/uuid.cpp")

if(${RATS_ENGINE_VULKAN_ENABLE})

	find_package(Vulkan REQUIRED)
	list(APPEND RATS_ENGINE_PRIVATE_LIBS Vulkan::Vulkan)
	list(APPEND RATS_ENGINE_PRIVATE_DEFINITIONS
		"VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"
		"VULKAN_HPP_NO_EXCEPTIONS=1"
		"VULKAN_HPP_NO_NODISCARD_WARNINGS=1")
	list(APPEND RATS_ENGINE_PRIVATE_HEADERS
		"private/engine/render/vulkan/core.h"
		"private/engine/render/vulkan/render_vulkan_service.h"
		"private/engine/render/vulkan/surface_vulkan_service.h"
		"private/engine/render/vulkan/builder/instance_builder.h"
		"private/engine/render/vulkan/builder/device_builder.h"
		"private/engine/render/vulkan/builder/swapchain_builder.h")
	list(APPEND RATS_ENGINE_SRC_FILES
		"src/render/vulkan/core.cpp"
		"src/render/vulkan/render_vulkan_service.cpp"
		"src/render/vulkan/surface_vulkan_service.cpp"
		"src/render/vulkan/builder/instance_builder.cpp"
		"src/render/vulkan/builder/device_builder.cpp"
		"src/render/vulkan/builder/swapchain_builder.cpp")

endif()

if(${RATS_ENGINE_GLFW_ENABLE})

	find_package(glfw3 CONFIG REQUIRED)
	list(APPEND RATS_ENGINE_PRIVATE_LIBS glfw)
	list(APPEND RATS_ENGINE_PRIVATE_HEADERS
		"private/engine/render/glfw/window_glfw_service.h")
	list(APPEND RATS_ENGINE_SRC_FILES
		"src/render/glfw/window_glfw_service.cpp")

	if(${RATS_ENGINE_VULKAN_ENABLE})
		list(APPEND RATS_ENGINE_PRIVATE_HEADERS
			"private/engine/render/glfw/window_glfw_vulkan_service.h")
		list(APPEND RATS_ENGINE_SRC_FILES
			"src/render/glfw/window_glfw_vulkan_service.cpp")
	endif()

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
configure_file(
	"cmake/configs/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/engine/config.h"
	@ONLY)
configure_file(
	"cmake/configs/private_config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/private/engine/private_config.h"
	@ONLY)
configure_file(
	"cmake/configs/private_config_macro.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/private/engine/private_config_macro.h"
	@ONLY)
target_include_directories(rats-engine
	PUBLIC "include" "${CMAKE_CURRENT_BINARY_DIR}/include"
	PRIVATE "private" "${CMAKE_CURRENT_BINARY_DIR}/private")

target_precompile_headers(rats-engine PRIVATE ${RATS_ENGINE_PRIVATE_PCH})
target_sources(rats-engine PUBLIC
	FILE_SET public_headers
		TYPE HEADERS
		BASE_DIRS "include"
		FILES ${RATS_ENGINE_PUBLIC_HEADERS})
target_sources(rats-engine PRIVATE
		FILE_SET private_headers
		TYPE HEADERS
		BASE_DIRS "private"
		FILES ${RATS_ENGINE_PRIVATE_HEADERS})
target_sources(rats-engine PRIVATE ${RATS_ENGINE_SRC_FILES} ${RATS_ENGINE_SRC_FILES_NO_PCH})
set_source_files_properties(${RATS_ENGINE_SRC_FILES_NO_PCH} PROPERTIES
	SKIP_PRECOMPILE_HEADERS ON)