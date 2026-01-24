#pragma once

#include "VulkanBuffer.h"
#include "VulkanLoader.h"
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

	struct PushConstants
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBufferAddress;
	};

	class RendererData
	{
	public:
		void Initialize();
		MeshBuffers UploadMesh(std::span<Vertex> vertices, std::span<uint32_t> indices);
		void CreatePipelines();
		void InitializeImmediate();
		void PrepareData();

	public:
		std::vector<std::shared_ptr<MeshAsset>> m_Meshes;
		std::shared_ptr<VulkanPipeline> m_MeshPipeline;
		std::shared_ptr<VulkanShader> m_VertexShader;
		std::shared_ptr<VulkanShader> m_FragmentShader;

		VkCommandPool m_Pool{ VK_NULL_HANDLE };
		VkCommandBuffer m_ImmediateCmdBuffer{ VK_NULL_HANDLE };
		VkFence m_ImmediateFence{ VK_NULL_HANDLE };
	};

}