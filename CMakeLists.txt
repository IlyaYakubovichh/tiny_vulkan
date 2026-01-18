cmake_minimum_required(VERSION 4.2.1)
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

file(GLOB_RECURSE includes 
    ${localRoot}/src/*.h
    ${localRoot}/src/*.hpp
)

source_group(TREE ${localRoot} FILES ${sources} ${includes})

# Target
add_executable(tiny_vulkan ${includes} ${sources}) 

target_include_directories(tiny_vulkan PUBLIC 
	${localRoot}/src/Vulkan
	${localRoot}/src/Window
    ${localRoot}/src/Core
)
install(TARGETS tiny_vulkan) # Using default GNUInstallDirs layout

# Dependencies
include(CMakeDeps.cmake)