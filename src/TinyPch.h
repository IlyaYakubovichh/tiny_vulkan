#pragma once

// I/O & Filesystem
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>

// Memory Management
#include <memory>
#include <new>

// Data Structures / Containers
#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <stack>
#include <queue>
#include <deque>
#include <span>       // C++20
#include <optional>   // C++17
#include <variant>    // C++17
#include <tuple>

// Algorithms & Functional
#include <algorithm>
#include <functional>
#include <utility>
#include <limits>
#include <numeric>
#include <iterator>
#include <random>
#include <chrono>
#include <ratio>

// Multithreading
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <atomic>
#include <shared_mutex>

// Error Handling & Type Traits
#include <stdexcept>
#include <cassert>
#include <type_traits>
#include <typeindex>

// C-style wrappers
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cfloat>


// =========================================================================
// GLM
// =========================================================================
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // (0..1 depth range)
#define GLM_ENABLE_EXPERIMENTAL     // gtx extensions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp> 

// =========================================================================
// Graphics API (Vulkan)
// =========================================================================
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// =========================================================================
// Common Project Macros
// =========================================================================
#define CHECK_VK_RES(smth)          \
    do {                            \
        VkResult err = (smth);      \
        if (err != VK_SUCCESS) {    \
            std::cerr << "VulkanError: " << string_VkResult(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            __debugbreak();         \
        } \
    } while(0)