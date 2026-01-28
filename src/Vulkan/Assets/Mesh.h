#pragma once

#include "VulkanBuffer.h"

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	struct Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
	};

	struct SubMeshGeo
	{
		uint32_t startIndex;
		uint32_t count;
	};

	struct Mesh
	{
		std::string name;

		std::vector<SubMeshGeo> subMeshesGeo;

		std::shared_ptr<VulkanBuffer> vertexBuffer;
		std::shared_ptr<VulkanBuffer> indexBuffer;

		VkDeviceAddress vertexBufferAddress;

		static std::shared_ptr<Mesh> CreateMeshFrom(
			const std::string& name, 
			const std::span<Vertex>& vertices,
			const std::span<uint32_t>& indices,
			const std::vector<SubMeshGeo>& subMeshesGeo);
	};

}