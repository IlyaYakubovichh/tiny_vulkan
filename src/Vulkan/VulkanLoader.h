#pragma once

#include "VulkanBuffer.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <filesystem>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace tiny_vulkan {

	struct Vertex { // std430 alignment is 16 bytes, but also need to align inside the struct
		glm::vec3 position;
		float uv_y;
		glm::vec3 normal;
		float uv_x;
		glm::vec4 color;
	};

	struct MeshBuffers
	{
		std::shared_ptr<VulkanBuffer> vertexBuffer;
		std::shared_ptr<VulkanBuffer> indexBuffer;
		VkDeviceAddress vertexBufferAddress;
	};

	struct GeoSurface
	{
		uint32_t startIndex;
		uint32_t count;
	};

	struct MeshAsset
	{
		std::string name;

		std::vector<GeoSurface> surfaces;
		MeshBuffers meshBuffers;
	};

	class Loader
	{
	public:
		static std::optional<std::vector<std::shared_ptr<MeshAsset>>> LoadGltfMeshes(std::filesystem::path filepath);
	};

}