#pragma once

#include "VulkanBuffer.h"
#include "VulkanDescriptors.h"
#include "VulkanPipelines.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <span>
#include <memory>
#include <vector>
#include <array>
#include <string>

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

	struct PushConstants
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBufferAddress;
	};

	class RendererData
	{
	public:
		void Initialize();

		auto GetImmediateCmdBuffer() const { return m_ImmediateCmdBuffer; }
		auto GetImmediateFence() const { return m_ImmediateFence; }
		auto GetPipeline() const { return m_MeshPipeline; }

		const auto& GetMeshBuffers() const { return m_MeshBuffers; }

	private:
		MeshBuffers UploadMesh(std::span<Vertex> vertices, std::span<uint32_t> indices);
		void CreatePipelines();
		void InitializeImmediate();
		void PrepareData();

	private:
		MeshBuffers m_MeshBuffers;

		std::shared_ptr<VulkanPipeline> m_MeshPipeline;
		std::shared_ptr<VulkanShader> m_VertexShader;
		std::shared_ptr<VulkanShader> m_FragmentShader;

		VkCommandPool m_Pool{ VK_NULL_HANDLE };
		VkCommandBuffer m_ImmediateCmdBuffer{ VK_NULL_HANDLE };
		VkFence m_ImmediateFence{ VK_NULL_HANDLE };
	};

}