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

option(RATS_ENGINE_WINDOW_GLFW "Use GLFW for window managing" OFF)

find_package(fmt CONFIG REQUIRED)
find_package(EASTL CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)


list(APPEND RATS_ENGINE_PUBLIC_LIBS 
	fmt::fmt 
	EASTL
	glm::glm)
list(APPEND RATS_ENGINE_PRIVATE_LIBS)

list(APPEND RATS_ENGINE_PUBLIC_MODULES
	"modules/engine.cppm"
	"modules/window.cppm"
	"modules/utils/utils.cppm"
	"modules/utils/log.cppm"
	"modules/utils/uuid.cppm")
list(APPEND RATS_ENGINE_PRIVATE_MODULES
	"private/window/contructor.cppm")
list(APPEND RATS_ENGINE_HEADERS
	"include/engine/macro/defer.h")
list(APPEND RATS_ENGINE_SRC_FILES
	"src/engine.cpp"
	"src/window/window.cpp"
	"src/utils/eastl_operator_new.cpp"
	"src/utils/log.cpp"
	"src/utils/uuid.cpp")

if(${RATS_ENGINE_WINDOW_GLFW})

	find_package(glfw3 CONFIG REQUIRED)
	list(APPEND RATS_ENGINE_PRIVATE_LIBS glfw)

	list(APPEND RATS_ENGINE_PRIVATE_MODULES
		"private/window/glfw/window.cppm")
	list(APPEND RATS_ENGINE_SRC_FILES
		"src/window/glfw/constructor.cpp"
		"src/window/glfw/window.cpp")

else()

	list(APPEND RATS_ENGINE_SRC_FILES
		"src/window/constructor.cpp")

endif()

add_library(rats-engine SHARED)
target_link_libraries(rats-engine PUBLIC ${RATS_ENGINE_PUBLIC_LIBS} PRIVATE ${RATS_ENGINE_PRIVATE_LIBS})

set_target_properties(rats-engine PROPERTIES 
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON)
generate_export_header(rats-engine EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/engine_export.h")
target_include_directories(rats-engine PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(rats-engine PUBLIC
	FILE_SET public_modules 
		TYPE CXX_MODULES
		BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/modules"
		FILES ${RATS_ENGINE_PUBLIC_MODULES})
target_sources(rats-engine PRIVATE
	FILE_SET private_modules 
		TYPE CXX_MODULES
		BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/private"
		FILES ${RATS_ENGINE_PRIVATE_MODULES})
target_sources(rats-engine PRIVATE ${RATS_ENGINE_HEADERS} ${RATS_ENGINE_SRC_FILES})
