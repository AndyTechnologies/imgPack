cmake_minimum_required(VERSION 4.0)
project(ImgPack LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ProjectLibrary ${PROJECT_NAME})

option(COMPILE_EXECUTABLE "Compilar el ejecutable de ejemplo" OFF)
option(USE_CCACHE "Utilizar ccache si está instalado" ON)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(USE_CCACHE)
	find_program(CCACHE_PROGRAM ccache)
	if(CCACHE_PROGRAM)
		message(STATUS "Usando ccache como lanzador de compilador: ${CCACHE_PROGRAM}")
		set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
	endif()
endif()


file(GLOB_RECURSE LibrarieSources CONFIGURE_DEPENDS 
    "lib/*.hpp" "lib/**/*.hpp"
    "lib/*.cpp" "lib/**/*.cpp"
    "lib/*.cc" "lib/**/*.cc"
    "lib/*.h" "lib/**/*.h"
    "lib/*.c" "lib/**/*.c"
)

include_directories(src)
include_directories(lib)
include_directories(./include)

add_custom_target(
    clangformat
    ALL
    COMMAND /usr/bin/clang-format-19
    -i
    ${LibrarieSources}
)

add_library(${ProjectLibrary} ${LibrarieSources})
target_precompile_headers(
	${ProjectLibrary} PRIVATE
	<cstdint>
	<string>
	<unordered_map>
	<vector>
	<cmath>
	<filesystem>
	<fstream>
	<print>
	<stdexcept>
)

if(COMPILE_EXECUTABLE)
	message(STATUS "Building Project Executable test.")
	set(ProjectTest ${PROJECT_NAME}-test)
	
	file(GLOB_RECURSE ProjectSources CONFIGURE_DEPENDS 
		"src/*.hpp" "src/**/*.hpp"
		"src/*.cpp" "src/**/*.cpp"
		"src/*.cc" "src/**/*.cc"
		"src/*.h" "src/**/*.h"
		"src/*.c" "src/**/*.c"
	)

	add_custom_target(
		clangformat
		ALL
		COMMAND /usr/bin/clang-format-19
		-i
		${ProjectSources}
	)

	add_executable(${ProjectTest} ${ProjectSources})
	target_link_libraries(${ProjectTest} PRIVATE ${ProjectLibrary})
endif()

# -------- Linking libs -----------
message(STATUS "Linking...")
include("cmake/Linking.cmake")