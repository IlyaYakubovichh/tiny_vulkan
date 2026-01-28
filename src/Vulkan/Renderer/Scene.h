#pragma once

#include "Mesh.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace tiny_vulkan {

	struct ScenePushConstants
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBufferAddress;
	};

	class Scene
	{
	public:
		Scene();
		~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		// Required commands in order to draw a scene (renderer will call it).
		void Render();

	private:
		ScenePushConstants m_ScenePushConstants;
		std::shared_ptr<VulkanPipeline> m_Pipeline;
		std::shared_ptr<VulkanShader> m_VertexShader;
		std::shared_ptr<VulkanShader> m_FragmentShader;
		std::vector<std::shared_ptr<Mesh>> m_Meshes;
	};

}