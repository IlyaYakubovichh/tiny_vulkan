#pragma once

#include "Mesh.h"

#include <filesystem>
#include <vector>
#include <optional>
#include <memory>

namespace tiny_vulkan::Loader {

	std::optional<std::vector<std::shared_ptr<Mesh>>> LoadGLTFMeshes(std::filesystem::path filepath);

}