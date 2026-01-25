#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace tiny_vulkan {

	class DescriptorLayoutBuilder
	{
	public:
		DescriptorLayoutBuilder() = default;

		DescriptorLayoutBuilder& AddBinding(uint32_t binding, uint32_t descriptorCount, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL);
		[[nodiscard]] VkDescriptorSetLayout Build(VkDevice device);

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
	};

}