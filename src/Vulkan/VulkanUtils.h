#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include "LogSystem.h"

#define CHECK_VK_RES(res)                                                                       \
    if(res != VK_SUCCESS) {                                                                     \
        LOG_CRITICAL(fmt::runtime("CRITICAL VULKAN ERROR: {0}"), string_VkResult(res));    \
        abort();                                                                                \
    }

namespace tiny_vulkan {

}