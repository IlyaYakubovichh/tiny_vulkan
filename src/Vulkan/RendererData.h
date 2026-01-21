#pragma once

#include "VulkanDescriptors.h"
#include "VulkanPipelines.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <array>
#include <string>

namespace tiny_vulkan {

	class RendererData
	{
	public:
		void Initialize();

		auto GetPipeline() const { return m_GraphicsPipeline; }
		
	private:
		void CreateShaders();
		void CreatePipeline();

	private:
		std::shared_ptr<VulkanPipeline>  m_GraphicsPipeline;
		std::shared_ptr<VulkanShader>	 m_VertexShader;
		std::shared_ptr<VulkanShader>	 m_FragmentShader;
	};

}