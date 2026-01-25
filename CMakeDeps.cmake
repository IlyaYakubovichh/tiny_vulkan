cmake_minimum_required(VERSION 4.2.1)
include(FetchContent)

# enable verbose output for FetchContent
set(FETCHCONTENT_QUIET OFF)


# ------------------------------------
# Vulkan library
# ------------------------------------
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
message(STATUS "Vulkan found: ${Vulkan_FOUND}")
message(STATUS "Vulkan include dirs: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan library: ${Vulkan_LIBRARY}")
message(STATUS "Vulkan volk library: ${Vulkan_volk_LIBRARY}")
message(STATUS "Vulkan shaderc_combined library: ${Vulkan_shaderc_combined_LIBRARY}")
target_link_options(tiny_vulkan INTERFACE LINKER:/IGNORE:4099)
target_link_libraries(tiny_vulkan PUBLIC 
	Vulkan::Vulkan
	Vulkan::shaderc_combined
)


# ------------------------------------
# GLFW library via FetchContent
# ------------------------------------
FetchContent_Declare( # content available immediately
  glfw
  URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(tiny_vulkan PUBLIC 
	glfw
)


# ------------------------------------
# Dear ImGui library via FetchContent
# ------------------------------------
FetchContent_Declare(
  imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.4.tar.gz
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_sources(imgui PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
target_link_libraries(imgui PRIVATE 
	glfw
	Vulkan::Vulkan
)
target_link_libraries(tiny_vulkan PUBLIC 
	imgui
)


# ------------------------------------
# spdlog library via FetchContent
# ------------------------------------
FetchContent_Declare(
  spdlog
  URL https://github.com/gabime/spdlog/archive/refs/tags/v1.16.0.tar.gz
)
FetchContent_MakeAvailable(spdlog)
target_link_libraries(tiny_vulkan PUBLIC 
	spdlog
)


# ------------------------------------
# vk-bootstrap library via FetchContent
# ------------------------------------
FetchContent_Declare(
  vkbootstrap
  URL https://github.com/charles-lunarg/vk-bootstrap/archive/refs/tags/v1.4.330.tar.gz
)
FetchContent_MakeAvailable(vkbootstrap)
target_link_libraries(tiny_vulkan PUBLIC 
	vk-bootstrap::vk-bootstrap
)


# ------------------------------------
# glm library via FetchContent
# ------------------------------------
FetchContent_Declare(
  glm
  URL https://github.com/g-truc/glm/archive/refs/tags/1.0.2.tar.gz
)
FetchContent_MakeAvailable(glm)
target_link_libraries(tiny_vulkan PUBLIC
	glm::glm
)

# ------------------------------------
# vma library
# ------------------------------------
FetchContent_Declare(
	vma
	URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.3.0.tar.gz
)
FetchContent_MakeAvailable(vma)
target_link_libraries(tiny_vulkan PUBLIC 
	GPUOpen::VulkanMemoryAllocator
)


# ------------------------------------
# fastgltf library 
# ------------------------------------
FetchContent_Declare(
    fastgltf
    URL https://github.com/spnda/fastgltf/archive/refs/tags/v0.9.0.tar.gz
)
FetchContent_MakeAvailable(fastgltf)
target_link_libraries(tiny_vulkan PUBLIC 
	fastgltf::fastgltf
)


# Finish
# Puts deps in Folder CMakeDeps in IDE
function(group_third_party target_name)
    if(TARGET ${target_name})
        set_target_properties(${target_name} PROPERTIES FOLDER "CMakeDeps")
    endif()
endfunction()


group_third_party(glfw)
group_third_party(imgui)
group_third_party(spdlog)
group_third_party(vk-bootstrap)
group_third_party(glm)
group_third_party(vma)
group_third_party(fastgltf)