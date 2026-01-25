#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <cstdint> 

#include "LogSystem.h"

namespace tiny_vulkan::IO {

    // Reads text files completely into memory.
    [[nodiscard]]
    std::optional<std::string> ReadFile(const std::filesystem::path& path);

    // Reads a binary files. Should be used for read from .spv file.
    [[nodiscard]]
    std::optional<std::vector<uint32_t>> ReadFileBin(const std::filesystem::path& path);

}