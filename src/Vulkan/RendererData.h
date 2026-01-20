#pragma once

#include "VulkanDescriptors.h"
#include "VulkanPipelines.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>

namespace tiny_vulkan {

	struct ComputePushConstants
	{
		glm::vec4 data1;
		glm::vec4 data2;
		glm::vec4 data3;
		glm::vec4 data4;
	};

	struct RendererData
	{
		ComputePushConstants pushConstants;

		VkDescriptorSetLayout					setLayout{ VK_NULL_HANDLE };
		std::shared_ptr<VulkanPipeline>			pipeline;
		std::shared_ptr<VulkanDescriptorPool>	desriptorPool;
		std::shared_ptr<VulkanDescriptorSet>	descriptorSet;
		std::shared_ptr<VulkanShader>			computeShader;

		void Initialize();
	};

}