cmake_minimum_required(VERSION 4.0.0)
project(tiny_vulkan)

# Root directory
set(localRoot ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Files
file(GLOB_RECURSE sources 
    ${localRoot}/src/*.cpp
    ${localRoot}/src/*.ixx
)

file(GLOB_RECURSE shaders
	${localRoot}/src/*.vert
	${localRoot}/src/*.frag
	${localRoot}/src/*.comp
)

file(GLOB_RECURSE includes 
    ${localRoot}/src/*.h
    ${localRoot}/src/*.hpp
)

source_group(TREE ${localRoot} FILES ${sources} ${includes} ${shaders})

# Target
add_executable(tiny_vulkan ${includes} ${sources} ${shaders}) 

if(MSVC)
	target_link_options(tiny_vulkan PRIVATE LINKER:/IGNORE:4099)
endif()

target_precompile_headers(tiny_vulkan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/TinyPch.h)

target_include_directories(tiny_vulkan PUBLIC 
	${localRoot}/src/Vulkan
	${localRoot}/src/Vulkan/Assets
	${localRoot}/src/Vulkan/Commands
	${localRoot}/src/Vulkan/Core
	${localRoot}/src/Vulkan/Pipeline
	${localRoot}/src/Vulkan/Renderer
	${localRoot}/src/Vulkan/Resources
	${localRoot}/src/Window
    ${localRoot}/src/Core
	${localRoot}/src/EntryPoint
)
install(TARGETS tiny_vulkan) # Using default GNUInstallDirs layout

# Dependencies
include(CMakeDeps.cmake)